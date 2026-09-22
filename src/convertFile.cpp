/***************************************************************************
**  vmm-essdat
**  Data analysis program for ESS RMM data (VMM3a, CAEN R5560, I-BM)
**
**  This program is free software: you can redistribute it and/or modify
**  it under the terms of the GNU General Public License as published by
**  the Free Software Foundation, either version 3 of the License, or
**  (at your option) any later version.
**
**  You should have received a copy of the GNU General Public License
**  along with this program.  If not, see http://www.gnu.org/licenses/.
**
****************************************************************************
**  Contact: dorothea.pfeiffer@cern.ch
**  Date: 12.10.2025
**  Version: 1.0.0
****************************************************************************
**
**  vmm-essdat
**  convertFile.cpp
**
****************************************************************************/

#include <chrono>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <log.h>
#include "Clusterer.h"
#include "Configuration.h"
#include <parser/CalibrationFile.h>
#include <parser/R5560Parser.h>
#include <parser/IBMParser.h>
#include <parser/CDTParser.h>
#include <parser/ReaderPcap.h>
#include <parser/VMM3Parser.h>

int main(int argc, char **argv) {
  corryvreckan::Log::addStream(std::cout);
  corryvreckan::Log::setSection("convertFile");
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
  } else {
    return -1;
  }
  
  corryvreckan::LogLevel log_level = corryvreckan::Log::getLevelFromString(m_config.pLogLevel);
  corryvreckan::Log::setReportingLevel(log_level);

  timeStart = std::chrono::system_clock::now();

  m_stats.CreateFECStats(m_config);
  m_stats.CreateClusterStats(m_config);
  m_stats.CreatePCAPStats(m_config);
 
  if (m_config.pUseBunchFile == true) {
    bunchFile = TFile::Open(m_config.pBunchFile.c_str(), "READ");
    if (!bunchFile || bunchFile->IsZombie()) {
      corryvreckan::Log::setSection("convertFile");
      LOG(ERROR) << "Error opening bunch file: " << m_config.pBunchFile;          
      return -1;
    }
    bunchTree = (TTree *)(bunchFile->Get(m_config.pBunchTree.c_str()));
    m_config.pMapPulsetimeIntensity.clear();
    m_config.pMapTriggertimeIntensity.clear();
    if (bunchTree != nullptr) {
      int64_t psTime = 0;
      float psIntensity = 0;
      int64_t lastTime = 0;
      bunchTree->SetBranchAddress(m_config.pBunchTimeVariable.c_str(), &psTime);
      bunchTree->SetBranchAddress(m_config.pBunchIntensityVariable.c_str(),
                                  &psIntensity);
      const Long64_t numTrigger = bunchTree->GetEntries();
      for (Long64_t n = 0; n < numTrigger; ++n) {
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

  char buffer[10000];
  Clusterer *m_Clusterer = new Clusterer(m_config, m_stats);

  VMM3Parser *parser = new VMM3Parser();
  R5560Parser *parser_r5560 = new R5560Parser();
  IBMParser *parser_ibm = new IBMParser();
  CDTParser *parser_cdt = new CDTParser();

  ReadoutParser readoutParser;
  CalibrationFile calfile(m_config.pCalFilename);     
  ReaderPcap pcap(m_config.pFileName);
  int ret = pcap.open();
  if (ret < 0) {
    corryvreckan::Log::setSection("convertFile");
    LOG(ERROR) << "Error opening pcapng file: " << m_config.pFileName << " (return value " << ret << ")";          
    return -1;
  }

  uint64_t pcappackets = 0;
  uint64_t goodFrames = 0;
  uint64_t badFrames = 0;

  int rdsize;
  bool doContinue = true;
  int64_t seqNumError = 0;
  int64_t pulse_time_ns = 0;
  int64_t previous_pulse_time_ns = 0;
  int64_t last_pulse_time_ns = 0;
  double pulseIntensity = 0;
  int64_t t0_correction = m_config.pTime0Correction;

  while (doContinue &&
         (rdsize = pcap.read((char *)&buffer, sizeof(buffer))) != -1) {
    if (rdsize == 0) {
      continue;
    }

    ret = readoutParser.validate((char *)&buffer,
                                 static_cast<uint32_t>(rdsize));

    if (seqNumError != readoutParser.Stats.ErrorSeqNum) {
      corryvreckan::Log::setSection("convertFile");
      LOG(TRACE) << "Sequence number error at packet " << pcappackets; 
    }
    seqNumError = readoutParser.Stats.ErrorSeqNum;

    pcappackets++;
    if (ret != ReadoutParser::OK) {
      badFrames++;
      continue;
    } else {
      goodFrames++;
    }
    
    if (readoutParser.Packet.version == 0) {
      if(t0_correction == -1) {
        t0_correction = static_cast<int64_t>(readoutParser.Packet.HeaderPtr0->PulseHigh);
      }
      pulse_time_ns =
          (static_cast<int64_t>(readoutParser.Packet.HeaderPtr0->PulseHigh) - t0_correction) * 1'000'000'000LL +
          static_cast<int64_t>(std::llround(static_cast<double>(readoutParser.Packet.HeaderPtr0->PulseLow) * m_config.pESSTime_ns));
      previous_pulse_time_ns =
          (static_cast<int64_t>(readoutParser.Packet.HeaderPtr0->PrevPulseHigh) - t0_correction) * 1'000'000'000LL +
          static_cast<int64_t>(std::llround(static_cast<double>(readoutParser.Packet.HeaderPtr0->PrevPulseLow) * m_config.pESSTime_ns));
 
      } else {
      if(t0_correction == -1) {
        t0_correction = static_cast<int64_t>(readoutParser.Packet.HeaderPtr1->PulseHigh);
      }
       pulse_time_ns =
         (static_cast<int64_t>(readoutParser.Packet.HeaderPtr1->PulseHigh) - t0_correction) * 1'000'000'000LL +
         static_cast<int64_t>(std::llround(static_cast<double>(readoutParser.Packet.HeaderPtr1->PulseLow) * m_config.pESSTime_ns));
      
       previous_pulse_time_ns =
          (static_cast<int64_t>(readoutParser.Packet.HeaderPtr1->PrevPulseHigh) - t0_correction) * 1'000'000'000LL +
          static_cast<int64_t>(std::llround(static_cast<double>(readoutParser.Packet.HeaderPtr1->PrevPulseLow) * m_config.pESSTime_ns));
 
     }
   
    if (pulse_time_ns - last_pulse_time_ns > 0) {
      m_stats.IncrementCounter("NumberOfTriggers", STATISTIC_FEN);
      last_pulse_time_ns = pulse_time_ns;

      if (m_config.pUseBunchFile == true) {
        auto itStart =
            m_config.pMapTriggertimeIntensity.upper_bound(pulse_time_ns);
        auto itEnd = m_config.pMapTriggertimeIntensity.lower_bound(
            pulse_time_ns + 1000000000);
        int cnt = 0;
        if (itStart != itEnd) {
          for (auto it = itStart; it != itEnd; ++it) {
            pulseIntensity = it->second;
            cnt++;
          }
          if (cnt == 1) {
            m_stats.IncrementCounter("NumberOfMatchedTriggers", STATISTIC_FEN);
            if (pulseIntensity >= 1.0E+7 && pulseIntensity < 1.0E+8) {
              m_stats.IncrementCounter("NumberOfBunches_1E7_1E8",
                                       STATISTIC_FEN);
              m_stats.IncrementCounter("IntensityOfBunches_1E7_1E8",
                                       STATISTIC_FEN,
                                       static_cast<uint64_t>(std::llround(pulseIntensity)));
            } else if (pulseIntensity >= 1.0E+8 && pulseIntensity < 1.0E+9) {
              m_stats.IncrementCounter("NumberOfBunches_1E8_1E9",
                                       STATISTIC_FEN);
              m_stats.IncrementCounter("IntensityOfBunches_1E8_1E9",
                                       STATISTIC_FEN,
                                       static_cast<uint64_t>(std::llround(pulseIntensity)));
            } else if (pulseIntensity >= 1.0E+9 && pulseIntensity < 1.0E+10) {
              m_stats.IncrementCounter("NumberOfBunches_1E9_1E10",
                                       STATISTIC_FEN);
              m_stats.IncrementCounter("IntensityOfBunches_1E9_1E10",
                                       STATISTIC_FEN,
                                       static_cast<uint64_t>(std::llround(pulseIntensity)));
            } else if (pulseIntensity >= 1.0E+10 && pulseIntensity < 1.0E+11) {
              m_stats.IncrementCounter("NumberOfBunches_1E10_1E11",
                                       STATISTIC_FEN);
              m_stats.IncrementCounter("IntensityOfBunches_1E10_1E11",
                                       STATISTIC_FEN,
                                       static_cast<uint64_t>(std::llround(pulseIntensity)));
            } else if (pulseIntensity >= 1.0E+11 && pulseIntensity < 1.0E+12) {
              m_stats.IncrementCounter("NumberOfBunches_1E11_1E12",
                                       STATISTIC_FEN);
              m_stats.IncrementCounter("IntensityOfBunches_1E11_1E12",
                                       STATISTIC_FEN,
                                       static_cast<uint64_t>(std::llround(pulseIntensity)));
            } else if (pulseIntensity >= 1.0E+12 && pulseIntensity < 1.0E+13) {
              m_stats.IncrementCounter("NumberOfBunches_1E12_1E13",
                                       STATISTIC_FEN);
              m_stats.IncrementCounter("IntensityOfBunches_1E12_1E13",
                                       STATISTIC_FEN,
                                       static_cast<uint64_t>(std::llround(pulseIntensity)));
            }
            m_config.pMapPulsetimeIntensity.emplace(
                std::make_pair(pulse_time_ns, pulseIntensity));
          } else {
            m_stats.IncrementCounter("NumberOfDoubleMatchedTriggers",
                                     STATISTIC_FEN);
            m_config.pMapPulsetimeIntensity.emplace(
                std::make_pair(pulse_time_ns, 0));
          }
        } else {
          m_stats.IncrementCounter("NumberOfUnMatchedTriggers", STATISTIC_FEN);
          m_config.pMapPulsetimeIntensity.emplace(std::make_pair(pulse_time_ns, 0));
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
      const int64_t parsed_hits =
          parser->parse(readoutParser.Packet.DataPtr,
                        readoutParser.Packet.DataLength);
      if (parsed_hits < 0) {
        LOG(ERROR) << "VMM parser returned negative hit count: " << parsed_hits;
        continue;
      }
      const std::size_t hits = static_cast<std::size_t>(parsed_hits);
      total_hits += static_cast<uint64_t>(hits);
      for (std::size_t i = 0; i < hits; ++i) {
        auto &hit = parser->Result[i];
        const uint8_t assisterId = static_cast<uint8_t>(
            static_cast<uint16_t>(hit.RingId / 2) *
                static_cast<uint16_t>(FENS_PER_RING) +
            static_cast<uint16_t>(hit.FENId));
     
        int64_t complete_timestamp = 
         (static_cast<int64_t>(hit.TimeHigh) - t0_correction) * 1'000'000'000LL +
         static_cast<int64_t>(std::llround(static_cast<double>(hit.TimeLow) * m_config.pESSTime_ns));
        
        uint16_t adc = hit.OTADC & 0x03ff;
        bool overThreshold = hit.OTADC & 0x8000;

        m_stats.IncrementCounter("ParserDataReadouts", assisterId, 1);
        auto calib = calfile.getCalibration(hit.RingId, hit.FENId, hit.VMM, hit.Channel);

        int64_t time_without_calib = 
         static_cast<int64_t>(std::llround(
          1.5 * m_config.pBCTime_ns - static_cast<double>(hit.TDC) * static_cast<double>(m_config.pTAC) / 255.0
        ));

        int64_t time_with_calib = 
         static_cast<int64_t>(std::llround(
          (1.5 * m_config.pBCTime_ns - static_cast<double>(hit.TDC) * static_cast<double>(m_config.pTAC) / 255.0 - calib.time_offset)/calib.time_slope
        ));

        if (calib.adc_slope == 0) {
          LOG(TRACE) << "Error in calibration file: adc_slope correction for assister " << (int)assisterId << ", chip " << (int)hit.VMM
                    << ", channel " << (int)hit.Channel
                    << " is 0!"; 
          calib.adc_slope = 1.0;
          calib.adc_offset = 0.0;
        }
        int64_t corrected_adc_value = static_cast<int64_t>(std::llround(
            (static_cast<double>(adc) - calib.adc_offset) * calib.adc_slope));
        if (corrected_adc_value > 1023) {
          LOG(TRACE) << "ADC value " << adc << " after correction "
                     << corrected_adc_value << ", larger than 1023!";
          corrected_adc_value = 1023;
        } else if (corrected_adc_value < 0) {
          LOG(TRACE) << "ADC value " << adc << " after correction "
                     << corrected_adc_value << ", smaller than 0!";
          corrected_adc_value = 0;
        }
        const uint16_t corrected_adc =
            static_cast<uint16_t>(corrected_adc_value);
        m_Clusterer->FillCalibHistos(assisterId, hit.VMM, hit.Channel, adc,
                                     corrected_adc, time_without_calib,
                                     time_with_calib);

        bool result = m_Clusterer->AnalyzeHits(
            complete_timestamp, assisterId, hit.VMM, hit.Channel, hit.BC,
            hit.TDC, corrected_adc, overThreshold,
            time_with_calib, hit.GEO, pulse_time_ns, previous_pulse_time_ns);
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
      const int64_t parsed_hits =
          parser_r5560->parse(readoutParser.Packet.DataPtr,
                       readoutParser.Packet.DataLength);
      if (parsed_hits < 0) {
        LOG(ERROR) << "R5560 parser returned negative hit count: " << parsed_hits;
        continue;
      }
      const std::size_t hits = static_cast<std::size_t>(parsed_hits);
      total_hits += static_cast<uint64_t>(hits);
      for (std::size_t i = 0; i < hits; ++i) {
        auto &hit = parser_r5560->Result[i];
        const uint16_t fenid =
            static_cast<uint16_t>(hit.RingId / 2) *
                static_cast<uint16_t>(FENS_PER_RING) +
            static_cast<uint16_t>(hit.FENId);

        int64_t complete_timestamp = 
         (static_cast<int64_t>(hit.TimeHigh) - t0_correction) * 1'000'000'000LL +
         static_cast<int64_t>(std::llround(static_cast<double>(hit.TimeLow) * m_config.pESSTime_ns));

         m_stats.IncrementCounter("ParserDataReadouts", fenid, 1);
        bool result = m_Clusterer->SaveHitsR5560(
            complete_timestamp, static_cast<uint8_t>(hit.RingId / 2), hit.FENId,
            hit.Group, hit.AmplitudeA, hit.AmplitudeB, hit.AmplitudeC,
            hit.AmplitudeD, hit.OM, hit.Counter, pulse_time_ns, previous_pulse_time_ns);
        if (result == false ||
            (total_hits >= m_config.nHits && m_config.nHits > 0)) {
          doContinue = false;
          break;
        }
      }
    }
    else if (m_config.pDataFormat == 0x10) {
      const int64_t parsed_hits =
          parser_ibm->parse(readoutParser.Packet.DataPtr,
                       readoutParser.Packet.DataLength);
      if (parsed_hits < 0) {
        LOG(ERROR) << "IBM parser returned negative hit count: " << parsed_hits;
        continue;
      }
      const std::size_t hits = static_cast<std::size_t>(parsed_hits);
      total_hits += static_cast<uint64_t>(hits);
      for (std::size_t i = 0; i < hits; ++i) {
		  auto &hit = parser_ibm->Result[i];
          const uint16_t fenid =
              static_cast<uint16_t>(hit.RingId / 2) *
                  static_cast<uint16_t>(FENS_PER_RING) +
              static_cast<uint16_t>(hit.FENId);
		
		  int64_t complete_timestamp = 
      (static_cast<int64_t>(hit.TimeHigh) - t0_correction) * 1'000'000'000LL +
      static_cast<int64_t>(std::llround(static_cast<double>(hit.TimeLow) * m_config.pESSTime_ns));

		  m_stats.IncrementCounter("ParserDataReadouts", fenid, 1);
		  bool result = m_Clusterer->SaveHitsIBM(
			complete_timestamp, static_cast<uint8_t>(hit.RingId / 2), hit.FENId,
            static_cast<uint8_t>(hit.Type), hit.ADC, pulse_time_ns,
            previous_pulse_time_ns);
		  if (result == false ||
			(total_hits >= m_config.nHits && m_config.nHits > 0)) {
			doContinue = false;
			break;
		  }
    	}
    }
    //CDT DREAM
    else if (m_config.pDataFormat == 0x60) {
      const int64_t parsed_hits =
          parser_cdt->parse(readoutParser.Packet.DataPtr,
                       readoutParser.Packet.DataLength);
      if (parsed_hits < 0) {
        LOG(ERROR) << "CDT parser returned negative hit count: " << parsed_hits;
        continue;
      }
      const std::size_t hits = static_cast<std::size_t>(parsed_hits);
      total_hits += static_cast<uint64_t>(hits);
      for (std::size_t i = 0; i < hits; ++i) {
		  auto &hit = parser_cdt->Result[i];
          const uint16_t fenid =
              static_cast<uint16_t>(hit.RingId / 2) *
                  static_cast<uint16_t>(FENS_PER_RING) +
              static_cast<uint16_t>(hit.FENId);
		  
      int64_t complete_timestamp = 
         (static_cast<int64_t>(hit.TimeHigh) - t0_correction) * 1'000'000'000LL +
         static_cast<int64_t>(std::llround(static_cast<double>(hit.TimeLow) * m_config.pESSTime_ns));

	
		  m_stats.IncrementCounter("ParserDataReadouts", fenid, 1);
		  bool result = m_Clusterer->SaveHitsCDT(
			complete_timestamp, static_cast<uint8_t>(hit.RingId / 2), hit.FENId,
			hit.OM, hit.UID, hit.Cathode, hit.Anode, pulse_time_ns, previous_pulse_time_ns);
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
                        m_stats.GetCounter("NumberOfTriggers", STATISTIC_FEN));

  const auto counterToUint64 = [](int64_t value) -> uint64_t {
    return value > 0 ? static_cast<uint64_t>(value) : 0ULL;
  };
  m_stats.IncrementCounter("ErrorBuffer", STATISTIC_FEN,
                           counterToUint64(readoutParser.Stats.ErrorBuffer));
  m_stats.IncrementCounter("ErrorSize", STATISTIC_FEN,
                           counterToUint64(readoutParser.Stats.ErrorSize));
  m_stats.IncrementCounter("ErrorVersion", STATISTIC_FEN,
                           counterToUint64(readoutParser.Stats.ErrorVersion));
  m_stats.IncrementCounter("ErrorCookie", STATISTIC_FEN,
                           counterToUint64(readoutParser.Stats.ErrorCookie));
  m_stats.IncrementCounter("ErrorPad", STATISTIC_FEN,
                           counterToUint64(readoutParser.Stats.ErrorPad));
  m_stats.IncrementCounter("ErrorOutputQueue", STATISTIC_FEN,
                           counterToUint64(readoutParser.Stats.ErrorOutputQueue));
  m_stats.IncrementCounter("ErrorTypeSubType", STATISTIC_FEN,
                           counterToUint64(readoutParser.Stats.ErrorTypeSubType));
  m_stats.IncrementCounter("ErrorSeqNum", STATISTIC_FEN,
                           counterToUint64(readoutParser.Stats.ErrorSeqNum));
  m_stats.IncrementCounter("ErrorTimeHigh", STATISTIC_FEN,
                           counterToUint64(readoutParser.Stats.ErrorTimeHigh));
  m_stats.IncrementCounter("ErrorTimeFrac", STATISTIC_FEN,
                           counterToUint64(readoutParser.Stats.ErrorTimeFrac));
  m_stats.IncrementCounter("HeartBeats", STATISTIC_FEN,
                           counterToUint64(readoutParser.Stats.HeartBeats));
  m_stats.IncrementCounter("GoodFrames", STATISTIC_FEN, goodFrames);
  m_stats.IncrementCounter("BadFrames", STATISTIC_FEN, badFrames);
  m_stats.IncrementCounter("TotalFrames", STATISTIC_FEN, pcappackets);

  m_stats.IncrementCounter("ParserErrorSize", STATISTIC_FEN,
                           counterToUint64(parser->Stats.ErrorSize));
  m_stats.IncrementCounter("ParserErrorRing", STATISTIC_FEN,
                           counterToUint64(parser->Stats.ErrorRing));
  m_stats.IncrementCounter("ParserErrorFEN", STATISTIC_FEN,
                           counterToUint64(parser->Stats.ErrorFEN));
  m_stats.IncrementCounter("ParserErrorDataLength", STATISTIC_FEN,
                           counterToUint64(parser->Stats.ErrorDataLength));
  m_stats.IncrementCounter("ParserErrorTimeFrac", STATISTIC_FEN,
                           counterToUint64(parser->Stats.ErrorTimeFrac));
  m_stats.IncrementCounter("ParserErrorBC", STATISTIC_FEN,
                           counterToUint64(parser->Stats.ErrorBC));
  m_stats.IncrementCounter("ParserErrorADC", STATISTIC_FEN,
                           counterToUint64(parser->Stats.ErrorADC));
  m_stats.IncrementCounter("ParserErrorVMM", STATISTIC_FEN,
                           counterToUint64(parser->Stats.ErrorVMM));
  m_stats.IncrementCounter("ParserErrorChannel", STATISTIC_FEN,
                           counterToUint64(parser->Stats.ErrorChannel));
  m_stats.IncrementCounter("ParserReadouts", STATISTIC_FEN,
                           counterToUint64(parser->Stats.Readouts));
  m_stats.IncrementCounter("ParserCalibReadouts", STATISTIC_FEN,
                           counterToUint64(parser->Stats.CalibReadouts));
  m_stats.IncrementCounter("ParserDataReadouts", STATISTIC_FEN,
                           counterToUint64(parser->Stats.DataReadouts));
  m_stats.IncrementCounter("ParserOverThreshold", STATISTIC_FEN,
                           counterToUint64(parser->Stats.OverThreshold));
  m_Clusterer->FinishAnalysis();

  delete m_Clusterer;

  timeEnd = std::chrono::system_clock::now();

  const int64_t elapsed_ms =
      std::chrono::duration_cast<std::chrono::milliseconds>(timeEnd - timeStart)
          .count();
  uint64_t hit_size = 160;
  if (m_config.pDataFormat >= 0x30 && m_config.pDataFormat <= 0x3C) {
    hit_size = 192;
  }
  corryvreckan::Log::setSection("convertFile");
  LOG(INFO) << "****************************************";
  LOG(INFO) << "Stats (analysis):";
  LOG(INFO) << "****************************************";
  LOG(INFO) << "Analysis time: " << std::setprecision(1) << std::fixed
            << static_cast<double>(elapsed_ms) << " ms";

  if (elapsed_ms > 0) {
    const double elapsed_ms_d = static_cast<double>(elapsed_ms);
    const double hit_rate =
        1000.0 * static_cast<double>(total_hits) / elapsed_ms_d;
    const double data_rate =
        1000.0 * static_cast<double>(total_hits) *
        static_cast<double>(hit_size) / elapsed_ms_d;

    LOG(INFO) << "Hit rate: " << std::scientific << hit_rate << " hit/s";
    LOG(INFO) << "Data rate: " << std::scientific << data_rate << " bit/s";
  } else {
    LOG(INFO) << "Hit rate: n/a (analysis time < 1 ms)";
    LOG(INFO) << "Data rate: n/a (analysis time < 1 ms)";
  }
  LOG(INFO) << "****************************************";
  
    if (m_config.pUseBunchFile == true) {    
      LOG(INFO) << m_stats.GetCounter("NumberOfBunches_1E7_1E8", STATISTIC_FEN)
                << " bunches (between 1E+7 and 1E+8 protons), with total "
                   "intensity of "
                << std::fixed << std::setprecision(12)
                << m_stats.GetCounter("IntensityOfBunches_1E7_1E8",
                                      STATISTIC_FEN)
                << " protons";
      LOG(INFO) << m_stats.GetCounter("NumberOfBunches_1E8_1E9", STATISTIC_FEN)
                << " bunches (between 1E+8 and 1E+9 protons), with total "
                   "intensity of "
                << std::fixed << std::setprecision(12)
                << m_stats.GetCounter("IntensityOfBunches_1E8_1E9",
                                      STATISTIC_FEN)
                << " protons";
      LOG(INFO) << m_stats.GetCounter("NumberOfBunches_1E9_1E10", STATISTIC_FEN)
                << " bunches (between 1E+9 and 1E+10 protons), with total "
                   "intensity of "
                << std::fixed << std::setprecision(12)
                << m_stats.GetCounter("IntensityOfBunches_1E9_1E10",
                                      STATISTIC_FEN)
                << " protons";
      LOG(INFO) << m_stats.GetCounter("NumberOfBunches_1E10_1E11",
                                      STATISTIC_FEN)
                << " bunches (between 1E+10 and 1E+11 protons), with total "
                   "intensity of "
                << std::fixed << std::setprecision(12)
                << m_stats.GetCounter("IntensityOfBunches_1E10_1E11",
                                      STATISTIC_FEN)
                << " protons";
      LOG(INFO) << m_stats.GetCounter("NumberOfBunches_1E11_1E12",
                                      STATISTIC_FEN)
                << " bunches (between 1E+11 and 1E+12 protons), with total "
                   "intensity of "
                << std::fixed << std::setprecision(12)
                << m_stats.GetCounter("IntensityOfBunches_1E11_1E12",
                                      STATISTIC_FEN)
                << " protons";
      LOG(INFO) << m_stats.GetCounter("NumberOfBunches_1E12_1E13",
                                      STATISTIC_FEN)
                << " bunches (between 1E+12 and 1E+13 protons), with total "
                   "intensity of "
                << std::fixed << std::setprecision(12)
                << m_stats.GetCounter("IntensityOfBunches_1E12_1E13",
                                      STATISTIC_FEN)
                << " protons";
      LOG(INFO) << "****************************************";
    }
  	pcap.printStats();
  
  return 0;
}
