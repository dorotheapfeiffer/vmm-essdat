#pragma once

#include "Configuration.h"

#include <cstddef>
#include <cstdint>
#include <map>
#include <string>
#include <utility>
#include <vector>

class Statistics {
public:
  Statistics() = default;
  ~Statistics() = default;

  void CreateClusterStats(Configuration &config);
  void CreateFECStats(Configuration &config);
  void CreatePCAPStats(Configuration &config);

  uint64_t GetStatsDetector(const std::string &stats,
                            uint8_t det,
                            std::size_t n);

  void SetStatsDetector(const std::string &stats,
                        uint8_t det,
                        double value);

  uint64_t GetStatsPlane(const std::string &stats,
                         std::pair<uint8_t, uint8_t> dp,
                         std::size_t n);

  void SetStatsPlane(const std::string &stats,
                     std::pair<uint8_t, uint8_t> dp,
                     double value);

  void IncrementCounter(const std::string &error,
                        uint16_t fecId,
                        uint64_t increment = 1);

  uint64_t GetCounter(const std::string &error,
                      uint16_t fecId);

  int64_t GetOldTriggerTimestamp(uint16_t fecId);
  void SetOldTriggerTimestamp(uint16_t fecId,
                              int64_t readoutTimestamp);

  int64_t GetFirstTriggerTimestamp(uint16_t fecId);
  void SetFirstTriggerTimestamp(uint16_t fecId,
                                int64_t readoutTimestamp);

  int64_t GetMaxTriggerTimestamp(uint16_t fecId);
  void SetMaxTriggerTimestamp(uint16_t fecId,
                              int64_t readoutTimestamp);

  uint64_t GetLastFrameCounter(uint16_t fecId);
  void SetLastFrameCounter(uint16_t fecId,
                           uint64_t frameCounter);

  int64_t GetLowestCommonTriggerTimestampDet(uint8_t det);
  void SetLowestCommonTriggerTimestampDet(uint8_t det,
                                          int64_t val);

  int64_t GetLowestCommonTriggerTimestampPlane(
      std::pair<uint8_t, uint8_t> dp);

  void SetLowestCommonTriggerTimestampPlane(
      std::pair<uint8_t, uint8_t> dp,
      int64_t val);

  void PrintClusterStats(Configuration &config);
  void PrintFECStats(Configuration &config);

  void StatsOutput(std::size_t n,
                   uint64_t val,
                   const std::string &stat,
                   uint64_t cnt,
                   uint64_t cnt0 = 0,
                   uint64_t cnt1 = 0);

private:
  double m_acq_time = 0.0;

  uint64_t cntTriggers = 0;

  // Histogram/statistics counters
  std::map<
      std::pair<std::pair<uint8_t, uint8_t>, std::string>,
      std::vector<uint64_t>>
      m_stats_plane;

  std::map<
      std::pair<uint8_t, std::string>,
      std::vector<uint64_t>>
      m_stats_detector;

  std::vector<std::string> m_stats_plane_names;
  std::vector<std::string> m_stats_detector_names;

  std::map<std::string, double> m_factors;
  std::map<std::string, double> m_limits;
  std::map<std::string, std::string> m_units;

  std::map<std::pair<uint16_t, std::string>, uint64_t> m_counters;
  std::vector<std::string> m_counter_names;

  // per plane
  std::map<std::pair<uint8_t, uint8_t>, int64_t>
      m_lowestCommonTriggerTimestamp_plane;

  // per detector
  std::map<uint8_t, int64_t>
      m_lowestCommonTriggerTimestamp_det;

  // per FEC
  std::map<uint16_t, int64_t> m_deltaTriggerTimestamp;
  std::map<uint16_t, int64_t> m_oldTriggerTimestamp;
  std::map<uint16_t, int64_t> m_maxTriggerTimestamp;
  std::map<uint16_t, int64_t> m_firstTriggerTimestamp;
  std::map<uint16_t, int64_t> m_lastTriggerTimestamp;

  std::map<uint16_t, uint64_t> m_lastFrameCounter;
};