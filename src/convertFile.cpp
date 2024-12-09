#include <chrono>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "Clusterer.h"
#include "Configuration.h"
#include <parser/CalibrationFile.h>
#include <parser/R5560Parser.h>
#include <parser/ReaderPcap.h>
#include <parser/Trace.h>
#include <parser/VMM3Parser.h>

int main(int argc, char **argv) {
  TFile *bunchFile = nullptr;
  TTree *bunchTree = nullptr;
  uint64_t total_hits = 0;
  std::chrono::time_point<std::chrono::system_clock> timeEnd, timeStart;

  Configuration m_config;
  Statistics m_stats;

  if (m_config.ParseCommandLine(argc, argv)) {
    if (!m_config.CreateMapping()) {
      return -1;
    }
    if (!m_config.CalculateTransform()) {
      return -1;
    }
    if (!m_config.pIsPcap) {
      return 0;
    }
  } else {
    return -1;
  }
  timeStart = std::chrono::system_clock::now();
  uint64_t last_time = 0;

  if (m_config.pShowStats) {
    m_stats.CreateClusterStats(m_config);
    if (m_config.pIsPcap) {
      m_stats.CreatePCAPStats(m_config);
    }
  }
  if (m_config.pUseBunchFile == true) {
    bunchFile = TFile::Open(m_config.pBunchFile.c_str(), "READ");
    if (!bunchFile || bunchFile->IsZombie()) {
      std::cout << "Error opening bunch file: " << m_config.pBunchFile
                << std::endl;
      return -1;
    }
    bunchTree = (TTree *)(bunchFile->Get(m_config.pBunchTree.c_str()));
    m_config.pMapPulsetimeIntensity.clear();
    m_config.pMapTriggertimeIntensity.clear();
    if (bunchTree != nullptr) {
      double psTime = 0;
      float psIntensity = 0;
      double lastTime = 0;
      bunchTree->SetBranchAddress(m_config.pBunchTimeVariable.c_str(), &psTime);
      bunchTree->SetBranchAddress(m_config.pBunchIntensityVariable.c_str(),
                                  &psIntensity);
      int numTrigger = bunchTree->GetEntries();
      for (int n = 0; n < numTrigger; n++) {
        bunchTree->GetEntry(n);
        if (psTime > lastTime) {
          m_config.pMapTriggertimeIntensity.emplace(
              std::make_pair(psTime, static_cast<double>(psIntensity)));
        }
        lastTime = psTime;
      }
    }
    if (bunchFile != nullptr) {
      bunchFile->Close();
      delete bunchFile;
      bunchFile = nullptr;
    }
  }
  double firstTime = 0;
  char buffer[10000];
  Clusterer *m_Clusterer = new Clusterer(m_config, m_stats);
  VMM3Parser *parser = new VMM3Parser();
  R5560Parser *parser_r5560 = new R5560Parser();

  ReadoutParser readoutParser;
  Gem::CalibrationFile calfile(m_config.pCalFilename);
  ReaderPcap pcap(m_config.pFileName);
  int ret = pcap.open();
  if (ret < 0) {
    std::cout << "Error opening file: " << m_config.pFileName
              << ": return value " << ret << std::endl;
    return -1;
  }

  uint64_t pcappackets = 0;
  uint64_t goodFrames = 0;
  uint64_t badFrames = 0;

  int rdsize;
  bool doContinue = true;
  long seqNumError = 0;
  double pulseTime = 0;
  uint64_t pulse_time_ns = 0;
  uint64_t trigger_time_ns = 0;
  double pulseIntensity = 0;
  double t0_correction = m_config.pTime0Correction;

  while (doContinue &&
         (rdsize = pcap.read((char *)&buffer, sizeof(buffer))) != -1) {
    if (rdsize == 0) {
      continue; // non udp data
    }

    int ret = readoutParser.validate((char *)&buffer, rdsize,
                                     ReadoutParser::Loki4Amp);

    if (seqNumError != readoutParser.Stats.ErrorSeqNum) {
      // printf("Sequence number error at packet %" PRIu64 "\n",
      // pcappackets);
    }
    seqNumError = readoutParser.Stats.ErrorSeqNum;

    if (m_config.pShowStats) {
      pcappackets++;
      if (ret != ReadoutParser::OK) {
        badFrames++;
        continue;
      } else {
        goodFrames++;
      }
    }

    double temp_pulseTime = 0;
    uint64_t temp_pulseTime_ns = 0;
    if (readoutParser.Packet.version == 0) {
      if (t0_correction == 0) {
        t0_correction = readoutParser.Packet.HeaderPtr0->PulseHigh;
      }

      temp_pulseTime =
          (readoutParser.Packet.HeaderPtr0->PulseHigh - t0_correction) *
              1.0E+09 +
          readoutParser.Packet.HeaderPtr0->PulseLow * m_config.pBCTime_ns * 0.5;
      temp_pulseTime_ns =
          readoutParser.Packet.HeaderPtr0->PulseHigh * 1.0E+09 +
          readoutParser.Packet.HeaderPtr0->PulseLow * m_config.pBCTime_ns * 0.5;
    } else {
      if (t0_correction == 0) {
        t0_correction = readoutParser.Packet.HeaderPtr1->PulseHigh;
      }
      temp_pulseTime =
          (readoutParser.Packet.HeaderPtr1->PulseHigh - t0_correction) *
              1.0E+09 +
          readoutParser.Packet.HeaderPtr1->PulseLow * m_config.pBCTime_ns * 0.5;
      temp_pulseTime_ns =
          readoutParser.Packet.HeaderPtr1->PulseHigh * 1.0E+09 +
          readoutParser.Packet.HeaderPtr1->PulseLow * m_config.pBCTime_ns * 0.5;
    }

    // Filter out pulse times that come directly after a valid pulse
    // time due to jitter
    double theTriggerTime = 0;
    if (temp_pulseTime - pulseTime > 1.0E+06) {
      m_stats.IncrementCounter("NumberOfTriggers", NUMFECS - 1);
      pulseTime = temp_pulseTime;
      pulse_time_ns = temp_pulseTime_ns;
      if (m_config.pUseBunchFile == true) {
        auto itStart =
            m_config.pMapTriggertimeIntensity.upper_bound(pulse_time_ns);
        auto itEnd = m_config.pMapTriggertimeIntensity.lower_bound(
            pulse_time_ns + 1000000000);
        int cnt = 0;
        if (itStart != itEnd) {
          for (auto it = itStart; it != itEnd; ++it) {
            theTriggerTime = it->first;
            pulseIntensity = it->second;
            cnt++;
          }
          if (cnt == 1) {
            m_stats.IncrementCounter("NumberOfMatchedTriggers", NUMFECS - 1);
            if (pulseIntensity >= 1.0E+7 && pulseIntensity < 1.0E+8) {
              m_stats.IncrementCounter("NumberOfBunches_1E7_1E8", NUMFECS - 1);
              m_stats.IncrementCounter("IntensityOfBunches_1E7_1E8",
                                       NUMFECS - 1, pulseIntensity);
            } else if (pulseIntensity >= 1.0E+8 && pulseIntensity < 1.0E+9) {
              m_stats.IncrementCounter("NumberOfBunches_1E8_1E9", NUMFECS - 1);
              m_stats.IncrementCounter("IntensityOfBunches_1E8_1E9",
                                       NUMFECS - 1, pulseIntensity);
            } else if (pulseIntensity >= 1.0E+9 && pulseIntensity < 1.0E+10) {
              m_stats.IncrementCounter("NumberOfBunches_1E9_1E10", NUMFECS - 1);
              m_stats.IncrementCounter("IntensityOfBunches_1E9_1E10",
                                       NUMFECS - 1, pulseIntensity);
            } else if (pulseIntensity >= 1.0E+10 && pulseIntensity < 1.0E+11) {
              m_stats.IncrementCounter("NumberOfBunches_1E10_1E11",
                                       NUMFECS - 1);
              m_stats.IncrementCounter("IntensityOfBunches_1E10_1E11",
                                       NUMFECS - 1, pulseIntensity);
            } else if (pulseIntensity >= 1.0E+11 && pulseIntensity < 1.0E+12) {
              m_stats.IncrementCounter("NumberOfBunches_1E11_1E12",
                                       NUMFECS - 1);
              m_stats.IncrementCounter("IntensityOfBunches_1E11_1E12",
                                       NUMFECS - 1, pulseIntensity);
            } else if (pulseIntensity >= 1.0E+12 && pulseIntensity < 1.0E+13) {
              m_stats.IncrementCounter("NumberOfBunches_1E12_1E13",
                                       NUMFECS - 1);
              m_stats.IncrementCounter("IntensityOfBunches_1E12_1E13",
                                       NUMFECS - 1, pulseIntensity);
            }
            m_config.pMapPulsetimeIntensity.emplace(
                std::make_pair(pulseTime, pulseIntensity));
            /*std::cout << pulse_time_ns << " ," << theTriggerTime << ", "
                      << theTriggerTime - pulse_time_ns << ", "
                      << pulseTime << ", " << pulseIntensity <<
               std::endl;*/
          } else {
            m_stats.IncrementCounter("NumberOfDoubleMatchedTriggers",
                                     NUMFECS - 1);
            m_config.pMapPulsetimeIntensity.emplace(
                std::make_pair(pulseTime, 0));
          }
        } else {
          m_stats.IncrementCounter("NumberOfUnMatchedTriggers", NUMFECS - 1);
          m_config.pMapPulsetimeIntensity.emplace(std::make_pair(pulseTime, 0));
        }
      }
    }
    // VMM
    // TREX 64 (0x40)
    // NMX 68 (0x44)
    // FREIA 72 (0x48)
    // TBL MB 73 (0x49)
    // ESTIA 76 (0x4C)
    if (m_config.pDataFormat >= 0x40 && m_config.pDataFormat <= 0x4C) {
      int hits = parser->parse(readoutParser.Packet.DataPtr,
                               readoutParser.Packet.DataLength);
      total_hits += hits;
      for (int i = 0; i < hits; i++) {
        auto &hit = parser->Result[i];
        uint16_t assisterId =
            static_cast<uint8_t>(hit.RingId / 2) * 16 + hit.FENId;
        if (firstTime == 0) {
          firstTime = hit.TimeHigh * 1.0E+09;
        }

        double complete_timestamp = 0;
        if (t0_correction == 0) {
          //  ESS time format use 64bit timestamp in nanoseconds
          //  To be able to use double as type for timestamp calculation,
          //  the timestamp has to be truncated to 52 bit
          //  The easiest to have a relative timestamp with respect to the
          //  start of the run
          complete_timestamp = hit.TimeHigh * 1.0E+09 - firstTime +
                               hit.TimeLow * m_config.pBCTime_ns * 0.5;
        } else {
          // If several files will be joined later, it is recommended to
          // just remove the most significant bits of the 64 bit timestamp
          complete_timestamp = hit.TimeHigh * 1.0E+09 -
                               t0_correction * 1.0E+09 +
                               hit.TimeLow * m_config.pBCTime_ns * 0.5;
        }

        uint16_t adc = hit.OTADC & 0x03ff;
        bool overThreshold = hit.OTADC & 0x8000;

        m_stats.IncrementCounter("ParserDataReadouts", assisterId, 1);
        auto calib = calfile.getCalibration(assisterId, hit.VMM, hit.Channel);
        double chiptime_corrected =
            (1.5 * m_config.pBCTime_ns -
             static_cast<double>(hit.TDC) * static_cast<double>(m_config.pTAC) /
                 255 -
             calib.time_offset) *
            calib.time_slope;

        if (calib.adc_slope == 0) {
          std::cout << "Error in calibration file: adc_slope correction "
                       "for assister "
                    << (int)assisterId << ", chip " << (int)hit.VMM
                    << ", channel " << (int)hit.Channel
                    << " is 0!\nIs that intentional?" << std::endl;
        }
        int corrected_adc = (adc - calib.adc_offset) * calib.adc_slope;

        if (corrected_adc > 1023) {
          DTRACE(DEB,
                 "After correction, ADC value larger than 1023 "
                 "(10bit)!\nUncorrected ADC value %d, uncorrected ADC "
                 "value %d\n",
                 adc, corrected_adc);
          corrected_adc = 1023;
        } else if (corrected_adc < 0) {
          DTRACE(DEB,
                 "After correction, ADC value smaller than 0!"
                 "\nUncorrected ADC value %d, uncorrected ADC "
                 "value %d\n",
                 adc, corrected_adc);
          corrected_adc = 0;
        }
        double timewalk_correction =
            calib.timewalk_d +
            (calib.timewalk_a - calib.timewalk_d) /
                (1 + pow(corrected_adc / calib.timewalk_c, calib.timewalk_b));

        double corrected_time = chiptime_corrected - timewalk_correction;

        double time_without_calib =
            (1.5 * m_config.pBCTime_ns -
             static_cast<double>(hit.TDC) * static_cast<double>(m_config.pTAC) /
                 255.0);

        double time_with_calib =
            (1.5 * m_config.pBCTime_ns -
             static_cast<double>(hit.TDC) * static_cast<double>(m_config.pTAC) /
                 255.0 -
             calib.time_offset) *
            calib.time_slope;

        m_Clusterer->FillCalibHistos(assisterId, hit.VMM, hit.Channel, adc,
                                     corrected_adc, time_without_calib,
                                     time_with_calib);

        bool result = m_Clusterer->AnalyzeHits(
            complete_timestamp, assisterId, hit.VMM, hit.Channel, hit.BC,
            hit.TDC, static_cast<uint16_t>(corrected_adc), overThreshold != 0,
            corrected_time, hit.GEO, pulseTime);
        if (result == false ||
            (total_hits >= m_config.nHits && m_config.nHits > 0)) {
          doContinue = false;
          break;
        }
      }
      // CAEN R5560
      //  Loki 0x30 (48)
      //  TBL He3 0x32 (50)
      //  BIFROST 0x34 (52)
      //  Miracles 0x38 (56)
      //  CSPEC 0x3C (60)
    } else if (m_config.pDataFormat >= 0x30 && m_config.pDataFormat <= 0x3C) {
      int hits = parser_r5560->parse(readoutParser.Packet.DataPtr,
                                     readoutParser.Packet.DataLength);
      total_hits += hits;
      for (int i = 0; i < hits; i++) {
        auto &hit = parser_r5560->Result[i];
        uint16_t fenid = static_cast<uint8_t>(hit.RingId / 2) * 16 + hit.FENId;
        if (firstTime == 0) {
          firstTime = hit.TimeHigh * 1.0E+09;
        }

        double complete_timestamp = 0;
        if (t0_correction == 0) {
          //  ESS time format use 64bit timestamp in nanoseconds
          //  To be able to use double as type for timestamp calculation,
          //  the timestamp has to be truncated to 52 bit
          //  The easiest to have a relative timestamp with respect to the
          //  start of the run
          complete_timestamp = hit.TimeHigh * 1.0E+09 - firstTime +
                               hit.TimeLow * m_config.pBCTime_ns * 0.5;
        } else {
          // If several files will be joined later, it is recommended to
          // just remove the most significant bits of the 64 bit timestamp
          complete_timestamp = hit.TimeHigh * 1.0E+09 -
                               t0_correction * 1.0E+09 +
                               hit.TimeLow * m_config.pBCTime_ns * 0.5;
        }

        m_stats.IncrementCounter("ParserDataReadouts", fenid, 1);
        bool result = m_Clusterer->SaveHitsR5560(
            complete_timestamp, hit.RingId, hit.FENId, hit.Group,
            hit.AmplitudeA, hit.AmplitudeB, hit.AmplitudeC, hit.AmplitudeD,
            hit.OM, hit.Counter, pulseTime);
        if (result == false ||
            (total_hits >= m_config.nHits && m_config.nHits > 0)) {
          doContinue = false;
          break;
        }
      }
    }
  }

  m_Clusterer->SaveDate(pcap.firstPacketSeconds, pcap.firstPacketDate,
                        pcap.lastPacketSeconds, pcap.lastPacketDate,
                        m_stats.GetCounter("NumberOfTriggers", NUMFECS - 1));
  m_stats.IncrementCounter("ErrorBuffer", NUMFECS - 1,
                           readoutParser.Stats.ErrorBuffer);
  m_stats.IncrementCounter("ErrorSize", NUMFECS - 1,
                           readoutParser.Stats.ErrorSize);
  m_stats.IncrementCounter("ErrorVersion", NUMFECS - 1,
                           readoutParser.Stats.ErrorVersion);
  m_stats.IncrementCounter("ErrorCookie", NUMFECS - 1,
                           readoutParser.Stats.ErrorCookie);
  m_stats.IncrementCounter("ErrorPad", NUMFECS - 1,
                           readoutParser.Stats.ErrorPad);
  m_stats.IncrementCounter("ErrorOutputQueue", NUMFECS - 1,
                           readoutParser.Stats.ErrorOutputQueue);
  m_stats.IncrementCounter("ErrorTypeSubType", NUMFECS - 1,
                           readoutParser.Stats.ErrorTypeSubType);
  m_stats.IncrementCounter("ErrorSeqNum", NUMFECS - 1,
                           readoutParser.Stats.ErrorSeqNum);
  m_stats.IncrementCounter("ErrorTimeHigh", NUMFECS - 1,
                           readoutParser.Stats.ErrorTimeHigh);
  m_stats.IncrementCounter("ErrorTimeFrac", NUMFECS - 1,
                           readoutParser.Stats.ErrorTimeFrac);
  m_stats.IncrementCounter("HeartBeats", NUMFECS - 1,
                           readoutParser.Stats.HeartBeats);
  m_stats.IncrementCounter("GoodFrames", NUMFECS - 1, goodFrames);
  m_stats.IncrementCounter("BadFrames", NUMFECS - 1, badFrames);
  m_stats.IncrementCounter("TotalFrames", NUMFECS - 1, pcappackets);

  m_stats.IncrementCounter("ParserErrorSize", NUMFECS - 1,
                           parser->Stats.ErrorSize);
  m_stats.IncrementCounter("ParserErrorRing", NUMFECS - 1,
                           parser->Stats.ErrorRing);
  m_stats.IncrementCounter("ParserErrorFEN", NUMFECS - 1,
                           parser->Stats.ErrorFEN);
  m_stats.IncrementCounter("ParserErrorDataLength", NUMFECS - 1,
                           parser->Stats.ErrorDataLength);
  m_stats.IncrementCounter("ParserErrorTimeFrac", NUMFECS - 1,
                           parser->Stats.ErrorTimeFrac);
  m_stats.IncrementCounter("ParserErrorBC", NUMFECS - 1, parser->Stats.ErrorBC);
  m_stats.IncrementCounter("ParserErrorADC", NUMFECS - 1,
                           parser->Stats.ErrorADC);
  m_stats.IncrementCounter("ParserErrorVMM", NUMFECS - 1,
                           parser->Stats.ErrorVMM);
  m_stats.IncrementCounter("ParserErrorChannel", NUMFECS - 1,
                           parser->Stats.ErrorChannel);
  m_stats.IncrementCounter("ParserReadouts", NUMFECS - 1,
                           parser->Stats.Readouts);
  m_stats.IncrementCounter("ParserCalibReadouts", NUMFECS - 1,
                           parser->Stats.CalibReadouts);
  m_stats.IncrementCounter("ParserDataReadouts", NUMFECS - 1,
                           parser->Stats.DataReadouts);
  m_stats.IncrementCounter("ParserOverThreshold", NUMFECS - 1,
                           parser->Stats.OverThreshold);
  m_Clusterer->FinishAnalysis();

  delete m_Clusterer;

  timeEnd = std::chrono::system_clock::now();

  int elapsed_seconds =
      std::chrono::duration_cast<std::chrono::milliseconds>(timeEnd - timeStart)
          .count();
  int hit_size = 160;
  if (m_config.pDataFormat >= 0x30 && m_config.pDataFormat <= 0x3C) {
    hit_size = 192;
  }

  std::cout << "\n****************************************" << std::endl;
  std::cout << "Stats (analysis):" << std::endl;
  std::cout << "****************************************" << std::endl;
  std::cout << "Analysis time: " << std::setprecision(1) << std::fixed
            << elapsed_seconds << " ms" << std::endl;
  std::cout << "Hit rate: " << std::scientific
            << static_cast<double>(1000 * total_hits / elapsed_seconds)
            << " hit/s" << std::endl;
  std::cout << "Data rate: " << std::scientific
            << static_cast<double>(1000 * total_hits * hit_size /
                                   elapsed_seconds)
            << " bit/s" << std::endl;
  std::cout << "****************************************" << std::endl;
  if (m_config.pUseBunchFile == true) {
    std::cout
        << m_stats.GetCounter("NumberOfBunches_1E7_1E8", NUMFECS - 1)
        << " bunches (between 1E+7 and 1E+8 protons), with total intensity of "
        << std::fixed << std::setprecision(12)
        << m_stats.GetCounter("IntensityOfBunches_1E7_1E8", NUMFECS - 1)
        << " protons" << std::endl;
    std::cout
        << m_stats.GetCounter("NumberOfBunches_1E8_1E9", NUMFECS - 1)
        << " bunches (between 1E+8 and 1E+9 protons), with total intensity of "
        << std::fixed << std::setprecision(12)
        << m_stats.GetCounter("IntensityOfBunches_1E8_1E9", NUMFECS - 1)
        << " protons" << std::endl;
    std::cout
        << m_stats.GetCounter("NumberOfBunches_1E9_1E10", NUMFECS - 1)
        << " bunches (between 1E+9 and 1E+10 protons), with total intensity of "
        << std::fixed << std::setprecision(12)
        << m_stats.GetCounter("IntensityOfBunches_1E9_1E10", NUMFECS - 1)
        << " protons" << std::endl;
    std::cout << m_stats.GetCounter("NumberOfBunches_1E10_1E11", NUMFECS - 1)
              << " bunches (between 1E+10 and 1E+11 protons), with total "
                 "intensity of "
              << std::fixed << std::setprecision(12)
              << m_stats.GetCounter("IntensityOfBunches_1E10_1E11", NUMFECS - 1)
              << " protons" << std::endl;
    std::cout << m_stats.GetCounter("NumberOfBunches_1E11_1E12", NUMFECS - 1)
              << " bunches (between 1E+11 and 1E+12 protons), with total "
                 "intensity of "
              << std::fixed << std::setprecision(12)
              << m_stats.GetCounter("IntensityOfBunches_1E11_1E12", NUMFECS - 1)
              << " protons" << std::endl;
    std::cout << m_stats.GetCounter("NumberOfBunches_1E12_1E13", NUMFECS - 1)
              << " bunches (between 1E+12 and 1E+13 protons), with total "
                 "intensity of "
              << std::fixed << std::setprecision(12)
              << m_stats.GetCounter("IntensityOfBunches_1E12_1E13", NUMFECS - 1)
              << " protons" << std::endl;
    std::cout << "****************************************" << std::endl;
  }
  return 0;
}
