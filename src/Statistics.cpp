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
**  Statistics.cpp
**
****************************************************************************/

#include <iomanip>
#include <iostream>
#include <log.h>
#include "Statistics.h"

void Statistics::CreatePCAPStats(Configuration &config) {

  m_counter_names.push_back("ErrorBuffer");
  m_counter_names.push_back("ErrorPad");
  m_counter_names.push_back("ErrorVersion");
  m_counter_names.push_back("ErrorCookie");
  m_counter_names.push_back("ErrorSize");
  m_counter_names.push_back("ErrorTypeSubType");
  m_counter_names.push_back("ErrorOutputQueue");
  m_counter_names.push_back("ErrorSeqNum");
  m_counter_names.push_back("ErrorTimeFrac");
  m_counter_names.push_back("HeartBeats");
  m_counter_names.push_back("TotalFrames");
  m_counter_names.push_back("BadFrames");
  m_counter_names.push_back("GoodFrames");
  m_counter_names.push_back("NumberOfTriggers");

  if (config.pUseBunchFile == true) {
    // Counters for n_TOF analysis
    m_counter_names.push_back("NumberOfDoubleMatchedTriggers");
    m_counter_names.push_back("NumberOfUnmatchedTriggers");
    m_counter_names.push_back("NumberOfMatchedTriggers");
    m_counter_names.push_back("NumberOfBunches_1E7_1E8");
    m_counter_names.push_back("NumberOfBunches_1E8_1E9");
    m_counter_names.push_back("NumberOfBunches_1E9_1E10");
    m_counter_names.push_back("NumberOfBunches_1E10_1E11");
    m_counter_names.push_back("NumberOfBunches_1E11_1E12");
    m_counter_names.push_back("NumberOfBunches_1E12_1E13");
    m_counter_names.push_back("IntensityOfBunches_1E7_1E8");
    m_counter_names.push_back("IntensityOfBunches_1E8_1E9");
    m_counter_names.push_back("IntensityOfBunches_1E9_1E10");
    m_counter_names.push_back("IntensityOfBunches_1E10_1E11");
    m_counter_names.push_back("IntensityOfBunches_1E11_1E12");
    m_counter_names.push_back("IntensityOfBunches_1E12_1E13");
  }

  m_counter_names.push_back("ParserErrorSize");
  m_counter_names.push_back("ParserErrorRing");
  m_counter_names.push_back("ParserErrorFEN");
  m_counter_names.push_back("ParserErrorDataLength");
  m_counter_names.push_back("ParserErrorTimeFrac");
  m_counter_names.push_back("ParserErrorBC");
  m_counter_names.push_back("ParserErrorADC");
  m_counter_names.push_back("ParserErrorVMM");
  m_counter_names.push_back("ParserErrorChannel");
  m_counter_names.push_back("ParserReadouts");
  m_counter_names.push_back("ParserCalibReadouts");
  m_counter_names.push_back("ParserDataReadouts");
  m_counter_names.push_back("ParserOverThreshold");

  for (auto const &fec : config.pFecs) {
    m_counters.emplace(
        std::make_pair(std::make_pair(fec, "ParserDataReadouts"), 0));
    if (fec == STATISTIC_FEN) {
      m_counters.emplace(std::make_pair(std::make_pair(fec, "ErrorBuffer"), 0));
      m_counters.emplace(std::make_pair(std::make_pair(fec, "ErrorPad"), 0));
      m_counters.emplace(
          std::make_pair(std::make_pair(fec, "ErrorVersion"), 0));
      m_counters.emplace(std::make_pair(std::make_pair(fec, "ErrorCookie"), 0));
      m_counters.emplace(std::make_pair(std::make_pair(fec, "ErrorSize"), 0));
      m_counters.emplace(
          std::make_pair(std::make_pair(fec, "ErrorTypeSubType"), 0));
      m_counters.emplace(
          std::make_pair(std::make_pair(fec, "ErrorOutputQueue"), 0));
      m_counters.emplace(std::make_pair(std::make_pair(fec, "ErrorSeqNum"), 0));
      m_counters.emplace(
          std::make_pair(std::make_pair(fec, "ErrorTimeFrac"), 0));
      m_counters.emplace(std::make_pair(std::make_pair(fec, "HeartBeats"), 0));
      m_counters.emplace(std::make_pair(std::make_pair(fec, "TotalFrames"), 0));
      m_counters.emplace(std::make_pair(std::make_pair(fec, "BadFrames"), 0));
      m_counters.emplace(std::make_pair(std::make_pair(fec, "GoodFrames"), 0));
      m_counters.emplace(
          std::make_pair(std::make_pair(fec, "ParserErrorSize"), 0));
      m_counters.emplace(
          std::make_pair(std::make_pair(fec, "ParserErrorRing"), 0));
      m_counters.emplace(
          std::make_pair(std::make_pair(fec, "ParserErrorFEN"), 0));
      m_counters.emplace(
          std::make_pair(std::make_pair(fec, "ParserErrorDataLength"), 0));
      m_counters.emplace(
          std::make_pair(std::make_pair(fec, "ParserErrorTimeFrac"), 0));
      m_counters.emplace(
          std::make_pair(std::make_pair(fec, "ParserErrorBC"), 0));
      m_counters.emplace(
          std::make_pair(std::make_pair(fec, "ParserErrorADC"), 0));
      m_counters.emplace(
          std::make_pair(std::make_pair(fec, "ParserErrorVMM"), 0));
      m_counters.emplace(
          std::make_pair(std::make_pair(fec, "ParserErrorChannel"), 0));
      m_counters.emplace(
          std::make_pair(std::make_pair(fec, "ParserReadouts"), 0));
      m_counters.emplace(
          std::make_pair(std::make_pair(fec, "ParserCalibReadouts"), 0));
      m_counters.emplace(
          std::make_pair(std::make_pair(fec, "ParserDataReadouts"), 0));
      m_counters.emplace(
          std::make_pair(std::make_pair(fec, "ParserOverThreshold"), 0));
    }
  }
}

void Statistics::CreateFECStats(Configuration &config) {
  cntTriggers = 0;
  m_counter_names.push_back("TimestampTooLarge");
  m_counter_names.push_back("TimestampOrderError");
  m_counter_names.push_back("TimestampOverflow");

  for (auto const &fec : config.pFecs) {
    m_deltaTriggerTimestamp.emplace(std::make_pair(fec, 0));
    m_oldTriggerTimestamp.emplace(std::make_pair(fec, 0));
    m_lastFrameCounter.emplace(std::make_pair(fec, 0));
    m_counters.emplace(
        std::make_pair(std::make_pair(fec, "TimestampTooLarge"), 0));

    m_counters.emplace(
        std::make_pair(std::make_pair(fec, "TimestampOrderError"), 0));

    m_counters.emplace(
        std::make_pair(std::make_pair(fec, "TimestampOverflow"), 0));
  }
}

void Statistics::CreateClusterStats(Configuration &config) {
  int size = 0;

  for (auto const &det : config.pDets) {
    auto plane0 = std::make_pair(det.first, 0);
    auto plane1 = std::make_pair(det.first, 1);

    // initialize timestamps
    m_lowestCommonTriggerTimestamp_det[det.first] = 0;
    m_lowestCommonTriggerTimestamp_plane[plane0] = 0;
    m_lowestCommonTriggerTimestamp_plane[plane1] = 0;

    if (det.second == 0) {
      m_units.emplace(std::make_pair("DeltaTimeHits", "ns"));
      m_factors.emplace(std::make_pair("DeltaTimeHits", 0.02));
      m_limits.emplace(
          std::make_pair("DeltaTimeHits", 1 + config.pDeltaTimeHits[det.first] *
                                                  m_factors["DeltaTimeHits"]));
      m_stats_plane_names.push_back("DeltaTimeHits");

      m_units.emplace(std::make_pair("MissingStripsCluster", "strips"));
      m_factors.emplace(std::make_pair("MissingStripsCluster", 1));
      m_limits.emplace(std::make_pair(
          "MissingStripsCluster", 1 + config.pMissingStripsCluster[det.first] *
                                          m_factors["MissingStripsCluster"]));
      m_stats_plane_names.push_back("MissingStripsCluster");

      m_units.emplace(std::make_pair("SpanClusterTime", "ns"));
      m_factors.emplace(std::make_pair("SpanClusterTime", 0.02));
      m_limits.emplace(std::make_pair("SpanClusterTime",
                                      1 + config.pSpanClusterTime[det.first] *
                                              m_factors["SpanClusterTime"]));
      m_stats_plane_names.push_back("SpanClusterTime");

      m_units.emplace(std::make_pair("ClusterSize", "strips"));
      m_factors.emplace(std::make_pair("ClusterSize", 1));
      m_limits.emplace(
          std::make_pair("ClusterSize", 1 + 64 * m_factors["ClusterSize"]));
      m_stats_plane_names.push_back("ClusterSize");

      m_units.emplace(std::make_pair("ClusterCntPlane", ""));
      m_factors.emplace(std::make_pair("ClusterCntPlane", 1));
      m_limits.emplace(std::make_pair("ClusterCntPlane", 1));
      m_stats_plane_names.push_back("ClusterCntPlane");

      m_units.emplace(std::make_pair("DeltaTimePlanes_0_1", "ns"));
      m_factors.emplace(std::make_pair("DeltaTimePlanes_0_1", 0.02));
      m_limits.emplace(std::make_pair(
          "DeltaTimePlanes_0_1", 1 + config.pDeltaTimePlanes[det.first] *
                                         m_factors["DeltaTimePlanes_0_1"]));
      m_stats_detector_names.push_back("DeltaTimePlanes_0_1");

      m_units.emplace(std::make_pair("ChargeRatio_0_1", "%"));
      m_factors.emplace(std::make_pair("ChargeRatio_0_1", 0.1));
      m_limits.emplace(std::make_pair("ChargeRatio_0_1", 11));
      m_stats_detector_names.push_back("ChargeRatio_0_1");

      m_units.emplace(std::make_pair("ChargeRatio_1_0", "%"));
      m_factors.emplace(std::make_pair("ChargeRatio_1_0", 0.1));
      m_limits.emplace(std::make_pair("ChargeRatio_1_0", 10));
      m_stats_detector_names.push_back("ChargeRatio_1_0");

      m_units.emplace(std::make_pair("ClusterCntDetector", ""));
      m_factors.emplace(std::make_pair("ClusterCntDetector", 1));
      m_limits.emplace(std::make_pair("ClusterCntDetector", 1));

      m_stats_detector_names.push_back("ClusterCntDetector");
    }
    size = static_cast<int>(m_limits["DeltaTimeHits"]);
    std::vector<long> v(size, 0);
    std::fill(v.begin(), v.end(), 0);
    m_stats_plane.emplace(
        std::make_pair(std::make_pair(plane0, "DeltaTimeHits"), v));
    m_stats_plane.emplace(
        std::make_pair(std::make_pair(plane1, "DeltaTimeHits"), v));

    size = static_cast<int>(m_limits["MissingStripsCluster"]);
    v.resize(size);
    std::fill(v.begin(), v.end(), 0);
    m_stats_plane.emplace(
        std::make_pair(std::make_pair(plane0, "MissingStripsCluster"), v));
    m_stats_plane.emplace(
        std::make_pair(std::make_pair(plane1, "MissingStripsCluster"), v));

    size = static_cast<int>(m_limits["SpanClusterTime"]);
    v.resize(size);
    std::fill(v.begin(), v.end(), 0);
    m_stats_plane.emplace(
        std::make_pair(std::make_pair(plane0, "SpanClusterTime"), v));
    m_stats_plane.emplace(
        std::make_pair(std::make_pair(plane1, "SpanClusterTime"), v));

    size = static_cast<int>(m_limits["ClusterSize"]);
    v.resize(size);
    std::fill(v.begin(), v.end(), 0);
    m_stats_plane.emplace(
        std::make_pair(std::make_pair(plane0, "ClusterSize"), v));
    m_stats_plane.emplace(
        std::make_pair(std::make_pair(plane1, "ClusterSize"), v));

    size = static_cast<int>(m_limits["ClusterCntPlane"]);
    v.resize(size);
    std::fill(v.begin(), v.end(), 0);
    m_stats_plane.emplace(
        std::make_pair(std::make_pair(plane0, "ClusterCntPlane"), v));
    m_stats_plane.emplace(
        std::make_pair(std::make_pair(plane1, "ClusterCntPlane"), v));

    size = static_cast<int>(m_limits["DeltaTimePlanes_0_1"]);
    v.resize(size);
    std::fill(v.begin(), v.end(), 0);
    m_stats_detector.emplace(
        std::make_pair(std::make_pair(det.first, "DeltaTimePlanes_0_1"), v));

    size = static_cast<int>(m_limits["ChargeRatio_0_1"]);
    v.resize(size);
    std::fill(v.begin(), v.end(), 0);
    m_stats_detector.emplace(
        std::make_pair(std::make_pair(det.first, "ChargeRatio_0_1"), v));

    size = static_cast<int>(m_limits["ChargeRatio_1_0"]);
    v.resize(size);
    std::fill(v.begin(), v.end(), 0);
    m_stats_detector.emplace(
        std::make_pair(std::make_pair(det.first, "ChargeRatio_1_0"), v));

    size = static_cast<int>(m_limits["ClusterCntDetector"]);
    v.resize(size);
    std::fill(v.begin(), v.end(), 0);
    m_stats_detector.emplace(
        std::make_pair(std::make_pair(det.first, "ClusterCntDetector"), v));
  }
}

long Statistics::GetStatsDetector(std::string stats, uint8_t det, int n) {
  if (n < m_limits[stats.c_str()]) {
    return m_stats_detector[std::make_pair(det, stats.c_str())][n];
  }
  return -1;
}

void Statistics::SetStatsDetector(std::string stats, uint8_t det,
                                  double value) {
  if (value * m_factors[stats] < m_limits[stats]) {
    m_stats_detector[std::make_pair(det, stats)]
                    [static_cast<unsigned int>(value * m_factors[stats])]++;
  } else {
    m_stats_detector[std::make_pair(det, stats)][m_limits[stats] - 1]++;
  }
}

long Statistics::GetStatsPlane(std::string stats,
                               std::pair<uint8_t, uint8_t> dp, int n) {
  if (n < m_limits[stats]) {
    return m_stats_plane[std::make_pair(dp, stats)][n];
  }
  return -1;
}

void Statistics::SetStatsPlane(std::string stats,
                               std::pair<uint8_t, uint8_t> dp, double value) {
  if (value * m_factors[stats] < m_limits[stats]) {
    m_stats_plane[std::make_pair(dp, stats)]
                 [static_cast<unsigned int>(value * m_factors[stats])]++;
  } else {
    m_stats_plane[std::make_pair(dp, stats)][m_limits[stats] - 1]++;
  }
}

void Statistics::IncrementCounter(std::string error, uint16_t fecId,
                                  uint64_t increment) {
  m_counters[std::make_pair(fecId, error)] += increment;
}

long Statistics::GetCounter(std::string error, uint16_t fecId) {
  return m_counters[std::make_pair(fecId, error)];
}

double Statistics::GetOldTriggerTimestamp(uint16_t fecId) {
  return m_oldTriggerTimestamp[fecId];
}

void Statistics::SetOldTriggerTimestamp(uint16_t fecId,
                                        double readoutTimestamp) {
  m_oldTriggerTimestamp[fecId] = readoutTimestamp;
}

double Statistics::GetFirstTriggerTimestamp(uint16_t fecId) {
  return m_firstTriggerTimestamp[fecId];
}

void Statistics::SetFirstTriggerTimestamp(uint16_t fecId,
                                          double readoutTimestamp) {
  m_firstTriggerTimestamp[fecId] = readoutTimestamp;
}

double Statistics::GetMaxTriggerTimestamp(uint16_t fecId) {
  return m_maxTriggerTimestamp[fecId];
}

void Statistics::SetMaxTriggerTimestamp(uint16_t fecId,
                                        double readoutTimestamp) {
  m_maxTriggerTimestamp[fecId] = readoutTimestamp;
}

uint64_t Statistics::GetLastFrameCounter(uint16_t fecId) {
  return m_lastFrameCounter[fecId];
}

void Statistics::SetLastFrameCounter(uint16_t fecId, uint64_t frameCounter) {
  m_lastFrameCounter[fecId] = frameCounter;
}

double Statistics::GetLowestCommonTriggerTimestampDet(uint8_t det) {
  return m_lowestCommonTriggerTimestamp_det[det];
}

void Statistics::SetLowestCommonTriggerTimestampDet(uint8_t det, double val) {
  m_lowestCommonTriggerTimestamp_det[det] = val;
}

double Statistics::GetLowestCommonTriggerTimestampPlane(
    std::pair<uint8_t, uint8_t> dp) {
  return m_lowestCommonTriggerTimestamp_plane[dp];
}

void Statistics::SetLowestCommonTriggerTimestampPlane(
    std::pair<uint8_t, uint8_t> dp, double val) {
  m_lowestCommonTriggerTimestamp_plane[dp] = val;
}

void Statistics::PrintClusterStats(Configuration &config) {
  long totalPlane0 = 0;
  long totalPlane1 = 0;
  long totalDetector = 0;
  bool bothPlanes = false;
  corryvreckan::Log::setSection("Statistics");
  for (auto const &det : config.pDets) {
    auto dp0 = std::make_pair(det.first, 0);
    auto dp1 = std::make_pair(det.first, 1);

    long cnt =
        m_stats_detector[std::make_pair(det.first, "ClusterCntDetector")][0];
    long cnt0 = 1;
    if (m_stats_plane[std::make_pair(dp0, "ClusterCntPlane")][0] > 0) {
      cnt0 = m_stats_plane[std::make_pair(dp0, "ClusterCntPlane")][0];
    }
    long cnt1 = 1;
    if (m_stats_plane[std::make_pair(dp1, "ClusterCntPlane")][0] > 0) {
      cnt1 = m_stats_plane[std::make_pair(dp1, "ClusterCntPlane")][0];
    }

  	LOG(INFO) << "****************************************";
  	LOG(INFO) << "Stats detector " << (int)det.first;
    LOG(INFO) << "****************************************";

    for (auto const &stat : m_stats_plane_names) {
      if (config.GetDetectorPlane(dp0) == true) {
        LOG(INFO) << "****************************************";
        LOG(INFO) << "Plane 0: " << stat;
        LOG(INFO) << "****************************************";
        std::vector<long> v = m_stats_plane[std::make_pair(dp0, stat)];
        for (unsigned int n = 0; n < static_cast<unsigned int>(m_limits[stat]);
             n++) {
          StatsOutput(n, v[n], stat, cnt0);
        }
      }
      if (config.GetDetectorPlane(dp1) == true) {
        LOG(INFO) << "****************************************";
        LOG(INFO) << "****************************************";
        LOG(INFO) << "Plane 1: " << stat;
        LOG(INFO) << "****************************************";
        std::vector<long> v = m_stats_plane[std::make_pair(dp1, stat)];
        for (unsigned int n = 0; n < static_cast<unsigned int>(m_limits[stat]);
             n++) {
          StatsOutput(n, v[n], stat, cnt1);
        }
      }
      LOG(INFO) <<"****************************************";
    }
    if (config.GetDetectorPlane(dp0) == true &&
        config.GetDetectorPlane(dp1) == true) {
      for (auto const &stat : m_stats_detector_names) {
        LOG(INFO) << "****************************************";
        LOG(INFO) <<  stat;
        LOG(INFO) <<  "****************************************";
        std::vector<long> v = m_stats_detector[std::make_pair(det.first, stat)];
        for (unsigned int n = 0; n < static_cast<unsigned int>(m_limits[stat]);
             n++) {
          StatsOutput(n, v[n], stat, cnt, cnt0, cnt1);
        }
        LOG(INFO) << "****************************************";
      }
    }
  }
}

void Statistics::PrintFECStats(Configuration &config) {
  corryvreckan::Log::setSection("Statistics");
  uint64_t total_min = 0;
  uint64_t total_max = 0;
  for (auto const &fec : config.pFecs) {
    if (fec < STATISTIC_FEN) {
      uint64_t first = GetFirstTriggerTimestamp(fec);
      uint64_t max = GetMaxTriggerTimestamp(fec);
      if(total_min == 0 || first < total_min) {
        total_min = first;
      }
      if(total_max == 0 || total_max < max) {
        total_max = max;
      }
      m_acq_time = 0;
      m_acq_time = (max - first) / 1000000.0;
      if (GetCounter("ParserDataReadouts", fec) > 0) {
        LOG(INFO) << "****************************************";
        LOG(INFO) << "RING " << (int)fec / 16 << ", FEN " << fec % 16
                 ;
        LOG(INFO) << "Stats (acquisition):";
        LOG(INFO) << "****************************************";
        LOG(INFO) << "Hits: " << GetCounter("ParserDataReadouts", fec)
                 ;
        LOG(INFO) << "Acq time: " << std::setprecision(1) << std::fixed
                  << m_acq_time << " ms";
        LOG(INFO) << "Hit rate FEN: " << std::scientific
                  << 1000 * GetCounter("ParserDataReadouts", fec) / m_acq_time
                  << " hit/s";
        LOG(INFO) << "Data rate FEN: " << std::scientific
                  << 1000 * GetCounter("ParserDataReadouts", fec) * 160 /
                         m_acq_time
                  << " bit/s";
        LOG(INFO) << "****************************************";
      }

    } else if (fec == STATISTIC_FEN) {
      uint64_t first = static_cast<uint64_t>(GetFirstTriggerTimestamp(fec));
      uint64_t max = static_cast<uint64_t>(GetMaxTriggerTimestamp(fec));

      m_acq_time = 0;
      m_acq_time = (total_max - total_min) / 1000000.0;
      LOG(INFO) <<"****************************************";
      LOG(INFO) <<"System wide stats";
      LOG(INFO) <<"****************************************";
      LOG(INFO) <<"start time: " << std::setprecision(18) << config.pTime0Correction*1E9+first; 
      LOG(INFO) <<"end time: " << std::setprecision(18) << config.pTime0Correction*1E9+max; 
      for (unsigned int n = 0; n < m_counter_names.size(); n++) {
        LOG(INFO) <<m_counter_names[n] << ": "
                  << GetCounter(m_counter_names[n], fec);
      }
      LOG(INFO) <<"****************************************";
      LOG(INFO) <<"Trigger rate: " << std::scientific << std::setprecision(2)
                << static_cast<double>(1000 *
                                       GetCounter("NumberOfTriggers", fec)) /
                       static_cast<double>(m_acq_time)
                << " trigger/s (total triggers: "
                << GetCounter("NumberOfTriggers", fec) << ")";
      LOG(INFO) <<"****************************************";
    }
  }

  if (config.pDataFormat >= 0x40 && config.pDataFormat <= 0x4C) {
    LOG(INFO) <<"****************************************";
    long cnt = 0;
    for (auto const &det : config.pDets) {
      cnt += GetStatsDetector("ClusterCntDetector", det.first, 0);
    }
    LOG(INFO) <<"Total Cluster rate: " << std::scientific
              << 1000 * cnt / m_acq_time << " particles/s";
    LOG(INFO) <<"****************************************";
  }
}

void Statistics::StatsOutput(int n, long val, std::string stat, long cnt,
                             long cnt0, long cnt1) {
  corryvreckan::Log::setSection("Statistics");
  if (m_limits[stat] > 1 && cnt > 0) {
    // if (cnt == 0)
    // cnt = 1;
    if (m_factors[stat] != 1) {
      LOG(INFO) <<static_cast<unsigned int>(n / m_factors[stat]) << "-"
                << static_cast<unsigned int>(n / m_factors[stat] +
                                             1 / m_factors[stat] - 1)
                << " " << m_units[stat] << ":  " << val << " ("
                << std::setprecision(1) << std::fixed
                << (100 * (double)val / (double)cnt) << " %)";
    } else {
      LOG(INFO) <<static_cast<unsigned int>(n / m_factors[stat]) << " "
                << m_units[stat] << ":  " << val << " (" << std::setprecision(1)
                << std::fixed << (100 * (double)val / (double)cnt) << " %)";
               
    }
  } else {
    if (cnt0 > 0 && cnt1 > 0) {
      LOG(INFO) <<val << " (common cluster in detector, "
                << std::setprecision(1) << (100 * (double)val / (double)cnt0)
                << " % plane 0, " << std::setprecision(1) << std::fixed
                << (100 * (double)val / (double)cnt1) << " % plane 1)"
               ;
    } else {
      LOG(INFO) <<val << " (" << std::setprecision(1) << std::fixed
                << (100 * (double)val / (double)cnt) << " %)";
    }
  }
}