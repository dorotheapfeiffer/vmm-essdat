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
**  Clusterer.h
**
****************************************************************************/

#pragma once
#include "RootFile.h"
#include "Statistics.h"
#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <vector>

class Clusterer {
public:
  Clusterer(Configuration &config, Statistics &stats);

  ~Clusterer();

  bool SaveHitsR5560(int64_t readoutTimestamp, uint8_t ringId, uint8_t fenId,
                     uint8_t groupId, uint16_t ampa, uint16_t ampb,
                     uint16_t ampc, uint16_t ampd, uint8_t om, uint32_t counter,
                     int64_t pulseTime, int64_t previousPulseTime);

  bool SaveHitsIBM(int64_t readoutTimestamp, uint8_t ringId, uint8_t fenId,
                     uint8_t type, uint32_t adc_raw,
                     int64_t pulseTime, int64_t previousPulseTime);
  
  bool SaveHitsCDT(int64_t readoutTimestamp, uint8_t ringId, uint8_t fenId,
                     uint8_t OM, uint8_t UID, uint8_t Cathode, uint8_t Anode,
                     int64_t pulseTime, int64_t previousPulseTime);
                                  
  // Analyzing and storing the hits
  bool AnalyzeHits(int64_t readoutTimestamp, uint8_t fecId, uint8_t vmmId,
                   uint8_t chNo, uint16_t bcid, uint16_t tdc, uint16_t adc,
                   bool overThresholdFlag, int64_t chipTime, uint8_t geoId = 0,
                   int64_t pulseTime = 0, int64_t previousPulseTime=0);

  // Analyzing and storing the clusters in plane 0 and 1
  void AnalyzeClustersPlane(std::pair<uint8_t, uint8_t> dp);

  // Select hits that are ready to be clustered in time
  bool ChooseHitsToBeClustered(std::pair<uint8_t, uint8_t> dp);

  bool ChooseClustersToBeMatched(std::pair<uint8_t, uint8_t> dp);

  // Matching the clusters that are common between detector planes
  void AnalyzeClustersDetector(uint8_t det);

  int ClusterByTime(std::pair<uint8_t, uint8_t> dp);
  int ClusterByStrip(std::pair<uint8_t, uint8_t> dp, ClusterContainer &cluster,
                     int64_t maxDeltaTime);

  void AlgorithmUTPC(size_t idx_min_largest_time, size_t idx_max_largest_time,
                     std::vector<uint16_t> &vADC, std::vector<uint16_t> &vStrips,
                     std::vector<int64_t> &vTimes, double &positionUTPC,
                     int64_t &timeUTPC, double &positionAlgo, int64_t &timeAlgo);

  int MatchClustersDetector(uint8_t det);

  void FinishAnalysis();

  void SaveDate(double the_seconds_start, std::string the_date_start,
                double the_seconds_end, std::string the_date_end,
                uint64_t num_triggers);

  void FillCalibHistos(uint16_t fec, uint8_t vmm, uint8_t ch, float adc,
                       float adc_corrected, int64_t chip_time,
                       int64_t chip_time_corrected);

private:
  Configuration &m_config;
  Statistics &m_stats;

  int m_hitNr = 0;

  int64_t last_time0 = 0;
  int64_t last_time1 = 0;

  uint8_t m_oldVmmId = 0;
  uint8_t m_oldFecId = 0;

  std::map<std::pair<uint8_t, uint8_t>, HitContainer> m_hits;
  std::map<std::pair<uint8_t, uint8_t>, HitContainer> m_hits_new;
  std::map<std::pair<uint8_t, uint8_t>, ClusterVectorPlane> m_clusters;
  std::map<std::pair<uint8_t, uint8_t>, ClusterVectorPlane> m_clusters_new;
  std::map<uint8_t, ClusterVectorDetector> m_clusters_detector;

  int m_cluster_id = 0;
  int m_cluster_detector_id = 0;

  RootFile *m_rootFile;
  
  int posTof = 0;
  int negTof = 0;
  int negPrevTof = 0;
 
};
