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

#include "Statistics.h"

#include <cstddef>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <log.h>
#include <string>
#include <utility>
#include <vector>

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

  if (config.pUseBunchFile) {
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

  for (const auto &fec : config.pFecs) {
    const uint16_t fecId = static_cast<uint16_t>(fec);

    m_counters.emplace(std::make_pair(
        std::make_pair(fecId, std::string("ParserDataReadouts")), uint64_t{0}));

    if (fecId == static_cast<uint16_t>(STATISTIC_FEN)) {
      m_counters.emplace(std::make_pair(
          std::make_pair(fecId, std::string("ErrorBuffer")), uint64_t{0}));
      m_counters.emplace(std::make_pair(
          std::make_pair(fecId, std::string("ErrorPad")), uint64_t{0}));
      m_counters.emplace(std::make_pair(
          std::make_pair(fecId, std::string("ErrorVersion")), uint64_t{0}));
      m_counters.emplace(std::make_pair(
          std::make_pair(fecId, std::string("ErrorCookie")), uint64_t{0}));
      m_counters.emplace(std::make_pair(
          std::make_pair(fecId, std::string("ErrorSize")), uint64_t{0}));
      m_counters.emplace(std::make_pair(
          std::make_pair(fecId, std::string("ErrorTypeSubType")), uint64_t{0}));
      m_counters.emplace(std::make_pair(
          std::make_pair(fecId, std::string("ErrorOutputQueue")), uint64_t{0}));
      m_counters.emplace(std::make_pair(
          std::make_pair(fecId, std::string("ErrorSeqNum")), uint64_t{0}));
      m_counters.emplace(std::make_pair(
          std::make_pair(fecId, std::string("ErrorTimeFrac")), uint64_t{0}));
      m_counters.emplace(std::make_pair(
          std::make_pair(fecId, std::string("HeartBeats")), uint64_t{0}));
      m_counters.emplace(std::make_pair(
          std::make_pair(fecId, std::string("TotalFrames")), uint64_t{0}));
      m_counters.emplace(std::make_pair(
          std::make_pair(fecId, std::string("BadFrames")), uint64_t{0}));
      m_counters.emplace(std::make_pair(
          std::make_pair(fecId, std::string("GoodFrames")), uint64_t{0}));
      m_counters.emplace(std::make_pair(
          std::make_pair(fecId, std::string("ParserErrorSize")), uint64_t{0}));
      m_counters.emplace(std::make_pair(
          std::make_pair(fecId, std::string("ParserErrorRing")), uint64_t{0}));
      m_counters.emplace(std::make_pair(
          std::make_pair(fecId, std::string("ParserErrorFEN")), uint64_t{0}));
      m_counters.emplace(std::make_pair(
          std::make_pair(fecId, std::string("ParserErrorDataLength")),
          uint64_t{0}));
      m_counters.emplace(std::make_pair(
          std::make_pair(fecId, std::string("ParserErrorTimeFrac")),
          uint64_t{0}));
      m_counters.emplace(std::make_pair(
          std::make_pair(fecId, std::string("ParserErrorBC")), uint64_t{0}));
      m_counters.emplace(std::make_pair(
          std::make_pair(fecId, std::string("ParserErrorADC")), uint64_t{0}));
      m_counters.emplace(std::make_pair(
          std::make_pair(fecId, std::string("ParserErrorVMM")), uint64_t{0}));
      m_counters.emplace(std::make_pair(
          std::make_pair(fecId, std::string("ParserErrorChannel")),
          uint64_t{0}));
      m_counters.emplace(std::make_pair(
          std::make_pair(fecId, std::string("ParserReadouts")), uint64_t{0}));
      m_counters.emplace(std::make_pair(
          std::make_pair(fecId, std::string("ParserCalibReadouts")),
          uint64_t{0}));
      m_counters.emplace(std::make_pair(
          std::make_pair(fecId, std::string("ParserDataReadouts")),
          uint64_t{0}));
      m_counters.emplace(std::make_pair(
          std::make_pair(fecId, std::string("ParserOverThreshold")),
          uint64_t{0}));
    }
  }
}

void Statistics::CreateFECStats(Configuration &config) {
  cntTriggers = 0;

  m_counter_names.push_back("TimestampTooLarge");
  m_counter_names.push_back("TimestampOrderError");
  m_counter_names.push_back("TimestampOverflow");

  for (const auto &fec : config.pFecs) {
    const uint16_t fecId = static_cast<uint16_t>(fec);

    m_deltaTriggerTimestamp.emplace(std::make_pair(fecId, int64_t{0}));
    m_oldTriggerTimestamp.emplace(std::make_pair(fecId, int64_t{0}));
    m_firstTriggerTimestamp.emplace(std::make_pair(fecId, int64_t{0}));
    m_maxTriggerTimestamp.emplace(std::make_pair(fecId, int64_t{0}));
    m_lastTriggerTimestamp.emplace(std::make_pair(fecId, int64_t{0}));
    m_lastFrameCounter.emplace(std::make_pair(fecId, uint64_t{0}));

    m_counters.emplace(std::make_pair(
        std::make_pair(fecId, std::string("TimestampTooLarge")), uint64_t{0}));
    m_counters.emplace(std::make_pair(
        std::make_pair(fecId, std::string("TimestampOrderError")), uint64_t{0}));
    m_counters.emplace(std::make_pair(
        std::make_pair(fecId, std::string("TimestampOverflow")), uint64_t{0}));
  }
}

void Statistics::CreateClusterStats(Configuration &config) {
  for (const auto &det : config.pDets) {
    const auto plane0 =
        std::make_pair(det.first, static_cast<uint8_t>(0));
    const auto plane1 =
        std::make_pair(det.first, static_cast<uint8_t>(1));

    // Initialize timestamps.
    m_lowestCommonTriggerTimestamp_det[det.first] = int64_t{0};
    m_lowestCommonTriggerTimestamp_plane[plane0] = int64_t{0};
    m_lowestCommonTriggerTimestamp_plane[plane1] = int64_t{0};

    if (det.second == 0) {
      m_units.emplace(std::make_pair("DeltaTimeHits", "ns"));
      m_factors.emplace(std::make_pair("DeltaTimeHits", 0.02));
      m_limits.emplace(std::make_pair(
          "DeltaTimeHits",
          1.0 + static_cast<double>(config.pDeltaTimeHits[det.first]) *
                    m_factors["DeltaTimeHits"]));
      m_stats_plane_names.push_back("DeltaTimeHits");

      m_units.emplace(std::make_pair("MissingStripsCluster", "strips"));
      m_factors.emplace(std::make_pair("MissingStripsCluster", 1.0));
      m_limits.emplace(std::make_pair(
          "MissingStripsCluster",
          1.0 + static_cast<double>(config.pMissingStripsCluster[det.first]) *
                    m_factors["MissingStripsCluster"]));
      m_stats_plane_names.push_back("MissingStripsCluster");

      m_units.emplace(std::make_pair("SpanClusterTime", "ns"));
      m_factors.emplace(std::make_pair("SpanClusterTime", 0.02));
      m_limits.emplace(std::make_pair(
          "SpanClusterTime",
          1.0 + static_cast<double>(config.pSpanClusterTime[det.first]) *
                    m_factors["SpanClusterTime"]));
      m_stats_plane_names.push_back("SpanClusterTime");

      m_units.emplace(std::make_pair("ClusterSize", "strips"));
      m_factors.emplace(std::make_pair("ClusterSize", 1.0));
      m_limits.emplace(std::make_pair(
          "ClusterSize", 1.0 + 64.0 * m_factors["ClusterSize"]));
      m_stats_plane_names.push_back("ClusterSize");

      m_units.emplace(std::make_pair("ClusterCntPlane", ""));
      m_factors.emplace(std::make_pair("ClusterCntPlane", 1.0));
      m_limits.emplace(std::make_pair("ClusterCntPlane", 1.0));
      m_stats_plane_names.push_back("ClusterCntPlane");

      m_units.emplace(std::make_pair("DeltaTimePlanes_0_1", "ns"));
      m_factors.emplace(std::make_pair("DeltaTimePlanes_0_1", 0.02));
      m_limits.emplace(std::make_pair(
          "DeltaTimePlanes_0_1",
          1.0 + static_cast<double>(config.pDeltaTimePlanes[det.first]) *
                    m_factors["DeltaTimePlanes_0_1"]));
      m_stats_detector_names.push_back("DeltaTimePlanes_0_1");

      m_units.emplace(std::make_pair("ChargeRatio_0_1", "%"));
      m_factors.emplace(std::make_pair("ChargeRatio_0_1", 0.1));
      m_limits.emplace(std::make_pair("ChargeRatio_0_1", 11.0));
      m_stats_detector_names.push_back("ChargeRatio_0_1");

      m_units.emplace(std::make_pair("ChargeRatio_1_0", "%"));
      m_factors.emplace(std::make_pair("ChargeRatio_1_0", 0.1));
      m_limits.emplace(std::make_pair("ChargeRatio_1_0", 10.0));
      m_stats_detector_names.push_back("ChargeRatio_1_0");

      m_units.emplace(std::make_pair("ClusterCntDetector", ""));
      m_factors.emplace(std::make_pair("ClusterCntDetector", 1.0));
      m_limits.emplace(std::make_pair("ClusterCntDetector", 1.0));
      m_stats_detector_names.push_back("ClusterCntDetector");
    }

    auto makeVector = [this](const std::string &name) {
      const auto size = static_cast<std::size_t>(m_limits[name]);
      return std::vector<uint64_t>(size, uint64_t{0});
    };

    m_stats_plane.emplace(
        std::make_pair(std::make_pair(plane0, "DeltaTimeHits"),
                       makeVector("DeltaTimeHits")));
    m_stats_plane.emplace(
        std::make_pair(std::make_pair(plane1, "DeltaTimeHits"),
                       makeVector("DeltaTimeHits")));

    m_stats_plane.emplace(
        std::make_pair(std::make_pair(plane0, "MissingStripsCluster"),
                       makeVector("MissingStripsCluster")));
    m_stats_plane.emplace(
        std::make_pair(std::make_pair(plane1, "MissingStripsCluster"),
                       makeVector("MissingStripsCluster")));

    m_stats_plane.emplace(
        std::make_pair(std::make_pair(plane0, "SpanClusterTime"),
                       makeVector("SpanClusterTime")));
    m_stats_plane.emplace(
        std::make_pair(std::make_pair(plane1, "SpanClusterTime"),
                       makeVector("SpanClusterTime")));

    m_stats_plane.emplace(
        std::make_pair(std::make_pair(plane0, "ClusterSize"),
                       makeVector("ClusterSize")));
    m_stats_plane.emplace(
        std::make_pair(std::make_pair(plane1, "ClusterSize"),
                       makeVector("ClusterSize")));

    m_stats_plane.emplace(
        std::make_pair(std::make_pair(plane0, "ClusterCntPlane"),
                       makeVector("ClusterCntPlane")));
    m_stats_plane.emplace(
        std::make_pair(std::make_pair(plane1, "ClusterCntPlane"),
                       makeVector("ClusterCntPlane")));

    m_stats_detector.emplace(
        std::make_pair(std::make_pair(det.first, "DeltaTimePlanes_0_1"),
                       makeVector("DeltaTimePlanes_0_1")));

    m_stats_detector.emplace(
        std::make_pair(std::make_pair(det.first, "ChargeRatio_0_1"),
                       makeVector("ChargeRatio_0_1")));

    m_stats_detector.emplace(
        std::make_pair(std::make_pair(det.first, "ChargeRatio_1_0"),
                       makeVector("ChargeRatio_1_0")));

    m_stats_detector.emplace(
        std::make_pair(std::make_pair(det.first, "ClusterCntDetector"),
                       makeVector("ClusterCntDetector")));
  }
}

uint64_t Statistics::GetStatsDetector(const std::string &stats,
                                      uint8_t det,
                                      std::size_t n) {
  const auto &values = m_stats_detector[std::make_pair(det, stats)];

  if (n < values.size()) {
    return values[n];
  }

  return uint64_t{0};
}

void Statistics::SetStatsDetector(const std::string &stats,
                                  uint8_t det,
                                  double value) {
  const double factor = m_factors[stats];
  const double limit = m_limits[stats];
  const double scaledValue = value * factor;

  auto &values = m_stats_detector[std::make_pair(det, stats)];

  if (scaledValue >= 0.0 && scaledValue < limit) {
    const auto bin = static_cast<std::size_t>(scaledValue);
    ++values[bin];
  } else if (!values.empty()) {
    ++values.back();
  }
}

uint64_t Statistics::GetStatsPlane(const std::string &stats,
                                   std::pair<uint8_t, uint8_t> dp,
                                   std::size_t n) {
  const auto &values = m_stats_plane[std::make_pair(dp, stats)];

  if (n < values.size()) {
    return values[n];
  }

  return uint64_t{0};
}

void Statistics::SetStatsPlane(const std::string &stats,
                               std::pair<uint8_t, uint8_t> dp,
                               double value) {
  const double factor = m_factors[stats];
  const double limit = m_limits[stats];
  const double scaledValue = value * factor;

  auto &values = m_stats_plane[std::make_pair(dp, stats)];

  if (scaledValue >= 0.0 && scaledValue < limit) {
    const auto bin = static_cast<std::size_t>(scaledValue);
    ++values[bin];
  } else if (!values.empty()) {
    ++values.back();
  }
}

void Statistics::IncrementCounter(const std::string &error,
                                  uint16_t fecId,
                                  uint64_t increment) {
  m_counters[std::make_pair(fecId, error)] += increment;
}

uint64_t Statistics::GetCounter(const std::string &error, uint16_t fecId) {
  return m_counters[std::make_pair(fecId, error)];
}

int64_t Statistics::GetOldTriggerTimestamp(uint16_t fecId) {
  return m_oldTriggerTimestamp[fecId];
}

void Statistics::SetOldTriggerTimestamp(uint16_t fecId,
                                        int64_t readoutTimestamp) {
  m_oldTriggerTimestamp[fecId] = readoutTimestamp;
}

int64_t Statistics::GetFirstTriggerTimestamp(uint16_t fecId) {
  return m_firstTriggerTimestamp[fecId];
}

void Statistics::SetFirstTriggerTimestamp(uint16_t fecId,
                                          int64_t readoutTimestamp) {
  m_firstTriggerTimestamp[fecId] = readoutTimestamp;
}

int64_t Statistics::GetMaxTriggerTimestamp(uint16_t fecId) {
  return m_maxTriggerTimestamp[fecId];
}

void Statistics::SetMaxTriggerTimestamp(uint16_t fecId,
                                        int64_t readoutTimestamp) {
  m_maxTriggerTimestamp[fecId] = readoutTimestamp;
}

uint64_t Statistics::GetLastFrameCounter(uint16_t fecId) {
  return m_lastFrameCounter[fecId];
}

void Statistics::SetLastFrameCounter(uint16_t fecId, uint64_t frameCounter) {
  m_lastFrameCounter[fecId] = frameCounter;
}

int64_t Statistics::GetLowestCommonTriggerTimestampDet(uint8_t det) {
  return m_lowestCommonTriggerTimestamp_det[det];
}

void Statistics::SetLowestCommonTriggerTimestampDet(uint8_t det, int64_t val) {
  m_lowestCommonTriggerTimestamp_det[det] = val;
}

int64_t Statistics::GetLowestCommonTriggerTimestampPlane(
    std::pair<uint8_t, uint8_t> dp) {
  return m_lowestCommonTriggerTimestamp_plane[dp];
}

void Statistics::SetLowestCommonTriggerTimestampPlane(
    std::pair<uint8_t, uint8_t> dp,
    int64_t val) {
  m_lowestCommonTriggerTimestamp_plane[dp] = val;
}

void Statistics::PrintClusterStats(Configuration &config) {
  corryvreckan::Log::setSection("Statistics");

  for (const auto &det : config.pDets) {
    const auto dp0 =
        std::make_pair(det.first, static_cast<uint8_t>(0));
    const auto dp1 =
        std::make_pair(det.first, static_cast<uint8_t>(1));

    const uint64_t cnt =
        m_stats_detector[std::make_pair(det.first, "ClusterCntDetector")]
            [0];

    uint64_t cnt0 = 1;
    const auto &plane0Counts =
        m_stats_plane[std::make_pair(dp0, "ClusterCntPlane")];
    if (!plane0Counts.empty() && plane0Counts[0] > 0) {
      cnt0 = plane0Counts[0];
    }

    uint64_t cnt1 = 1;
    const auto &plane1Counts =
        m_stats_plane[std::make_pair(dp1, "ClusterCntPlane")];
    if (!plane1Counts.empty() && plane1Counts[0] > 0) {
      cnt1 = plane1Counts[0];
    }

    if (cnt == 0) {
      continue;
    }

    LOG(INFO) << "****************************************";
    LOG(INFO) << "Stats detector " << static_cast<int>(det.first);
    LOG(INFO) << "****************************************";

    for (const auto &stat : m_stats_plane_names) {
      if (config.GetDetectorPlane(dp0)) {
        LOG(INFO) << "****************************************";
        LOG(INFO) << "Plane 0: " << stat;
        LOG(INFO) << "****************************************";

        const auto &v = m_stats_plane[std::make_pair(dp0, stat)];
        for (std::size_t n = 0; n < v.size(); ++n) {
          StatsOutput(n, v[n], stat, cnt0);
        }
      }

      if (config.GetDetectorPlane(dp1)) {
        LOG(INFO) << "****************************************";
        LOG(INFO) << "Plane 1: " << stat;
        LOG(INFO) << "****************************************";

        const auto &v = m_stats_plane[std::make_pair(dp1, stat)];
        for (std::size_t n = 0; n < v.size(); ++n) {
          StatsOutput(n, v[n], stat, cnt1);
        }
      }

      LOG(INFO) << "****************************************";
    }

    if (config.GetDetectorPlane(dp0) && config.GetDetectorPlane(dp1)) {
      for (const auto &stat : m_stats_detector_names) {
        LOG(INFO) << "****************************************";
        LOG(INFO) << stat;
        LOG(INFO) << "****************************************";

        const auto &v = m_stats_detector[std::make_pair(det.first, stat)];
        for (std::size_t n = 0; n < v.size(); ++n) {
          StatsOutput(n, v[n], stat, cnt, cnt0, cnt1);
        }

        LOG(INFO) << "****************************************";
      }
    }
  }
}

void Statistics::PrintFECStats(Configuration &config) {
  corryvreckan::Log::setSection("Statistics");

  int64_t totalMin = 0;
  int64_t totalMax = 0;

  for (const auto &fec : config.pFecs) {
    const uint16_t fecId = static_cast<uint16_t>(fec);

    if (fecId < static_cast<uint16_t>(STATISTIC_FEN)) {
      const int64_t first = GetFirstTriggerTimestamp(fecId);
      const int64_t max = GetMaxTriggerTimestamp(fecId);

      if (totalMin == 0 || first < totalMin) {
        totalMin = first;
      }

      if (totalMax == 0 || totalMax < max) {
        totalMax = max;
      }

      m_acq_time = static_cast<double>(max - first) / 1'000'000.0;

      if (GetCounter("ParserDataReadouts", fecId) > 0) {
        LOG(INFO) << "****************************************";
        LOG(INFO) << "RING " << static_cast<unsigned int>(fecId / 16U)
                  << ", FEN " << static_cast<unsigned int>(fecId % 16U);
        LOG(INFO) << "Stats (acquisition):";
        LOG(INFO) << "****************************************";
        LOG(INFO) << "Hits: " << GetCounter("ParserDataReadouts", fecId);
        LOG(INFO) << "Acq time: " << std::setprecision(1) << std::fixed
                  << m_acq_time << " ms";

        if (m_acq_time > 0.0) {
          const double hitRate =
              1000.0 *
              static_cast<double>(GetCounter("ParserDataReadouts", fecId)) /
              m_acq_time;

          const double dataRate =
              1000.0 *
              static_cast<double>(GetCounter("ParserDataReadouts", fecId)) *
              160.0 / m_acq_time;

          LOG(INFO) << "Hit rate FEN: " << std::scientific << hitRate
                    << " hit/s";
          LOG(INFO) << "Data rate FEN: " << std::scientific << dataRate
                    << " bit/s";
        }

        LOG(INFO) << "****************************************";
      }

    } else if (fecId == static_cast<uint16_t>(STATISTIC_FEN)) {
      const int64_t first = GetFirstTriggerTimestamp(fecId);
      const int64_t max = GetMaxTriggerTimestamp(fecId);

      m_acq_time = static_cast<double>(totalMax - totalMin) / 1'000'000.0;

      LOG(INFO) << "****************************************";
      LOG(INFO) << "System wide stats";
      LOG(INFO) << "****************************************";

      const long double startTimeNs =
          static_cast<long double>(config.pTime0Correction) * 1.0e9L +
          static_cast<long double>(first);
      const long double endTimeNs =
          static_cast<long double>(config.pTime0Correction) * 1.0e9L +
          static_cast<long double>(max);

      LOG(INFO) << "start time: " << std::setprecision(18) << startTimeNs;
      LOG(INFO) << "end time: " << std::setprecision(18) << endTimeNs;

      for (std::size_t n = 0; n < m_counter_names.size(); ++n) {
        LOG(INFO) << m_counter_names[n] << ": "
                  << GetCounter(m_counter_names[n], fecId);
      }

      LOG(INFO) << "****************************************";

      if (m_acq_time > 0.0) {
        const double triggerRate =
            1000.0 *
            static_cast<double>(GetCounter("NumberOfTriggers", fecId)) /
            m_acq_time;

        LOG(INFO) << "Trigger rate: " << std::scientific
                  << std::setprecision(2) << triggerRate
                  << " trigger/s (total triggers: "
                  << GetCounter("NumberOfTriggers", fecId) << ")";
      }

      LOG(INFO) << "****************************************";
    }
  }

  if (config.pDataFormat >= 0x40 && config.pDataFormat <= 0x4C) {
    LOG(INFO) << "****************************************";

    uint64_t cnt = 0;
    for (const auto &det : config.pDets) {
      cnt += GetStatsDetector("ClusterCntDetector", det.first, 0);
    }

    if (m_acq_time > 0.0) {
      const double clusterRate =
          1000.0 * static_cast<double>(cnt) / m_acq_time;

      LOG(INFO) << "Total Cluster rate: " << std::scientific
                << clusterRate << " particles/s";
    }

    LOG(INFO) << "****************************************";
  }
}

void Statistics::StatsOutput(std::size_t n,
                             uint64_t val,
                             const std::string &stat,
                             uint64_t cnt,
                             uint64_t cnt0,
                             uint64_t cnt1) {
  corryvreckan::Log::setSection("Statistics");

  const double factor = m_factors[stat];
  const double limit = m_limits[stat];

  if (limit > 1.0 && cnt > 0) {
    if (factor != 1.0) {
      const auto low = static_cast<std::size_t>(
          static_cast<double>(n) / factor);
      const auto high = static_cast<std::size_t>(
          static_cast<double>(n) / factor + 1.0 / factor - 1.0);

      const double percentage =
          100.0 * static_cast<double>(val) / static_cast<double>(cnt);

      LOG(INFO) << low << "-" << high << " " << m_units[stat] << ":  "
                << val << " (" << std::setprecision(1) << std::fixed
                << percentage << " %)";
    } else {
      const auto bin = static_cast<std::size_t>(
          static_cast<double>(n) / factor);

      const double percentage =
          100.0 * static_cast<double>(val) / static_cast<double>(cnt);

      LOG(INFO) << bin << " " << m_units[stat] << ":  " << val << " ("
                << std::setprecision(1) << std::fixed << percentage << " %)";
    }
  } else {
    if (cnt0 > 0 && cnt1 > 0) {
      const double percentage0 =
          100.0 * static_cast<double>(val) / static_cast<double>(cnt0);
      const double percentage1 =
          100.0 * static_cast<double>(val) / static_cast<double>(cnt1);

      LOG(INFO) << val << " (common cluster in detector, "
                << std::setprecision(1) << std::fixed << percentage0
                << " % plane 0, " << std::setprecision(1) << std::fixed
                << percentage1 << " % plane 1)";
    } else if (cnt > 0) {
      const double percentage =
          100.0 * static_cast<double>(val) / static_cast<double>(cnt);

      LOG(INFO) << val << " (" << std::setprecision(1) << std::fixed
                << percentage << " %)";
    }
  }
}
