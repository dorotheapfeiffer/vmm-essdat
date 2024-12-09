#include "Clusterer.h"
#include <algorithm>
#include <cmath>
#include <parser/Trace.h>

#include <chrono>
#include <functional>
#include <future>
#include <iomanip>
#include <thread>
#define UNUSED __attribute__((unused))

//#undef TRC_LEVEL
//#define TRC_LEVEL TRC_L_DEB

auto now = std::chrono::steady_clock::now;

auto timethis(std::function<void()> thunk)
    -> decltype((now() - now()).count()) {
  auto start = now();
  thunk();
  auto stop = now();
  return (stop - start).count();
}

Clusterer::Clusterer(Configuration &config, Statistics &stats)
    : m_config(config), m_stats(stats) {
  m_rootFile = RootFile::GetInstance(config);
  m_pulseTime[0] = 0;
  m_pulseTime[1] = 0;
  m_pulseTime[2] = 0;
}

Clusterer::~Clusterer() { RootFile::Dispose(); }

//====================================================================================================================
bool Clusterer::SaveHitsR5560(double readoutTimestamp, uint8_t ringId,
                              uint8_t fenId, uint8_t groupId, uint16_t ampa,
                              uint16_t ampb, uint16_t ampc, uint16_t ampd,
                              uint8_t om, uint32_t counter, double pulseTime) {

  bool newData = false;
  if (m_stats.GetFirstTriggerTimestamp(NUMFECS - 1) == 0) {
    m_stats.SetFirstTriggerTimestamp(NUMFECS - 1, readoutTimestamp);
  }
  if (m_stats.GetFirstTriggerTimestamp(ringId * 16 + fenId) == 0) {
    m_stats.SetFirstTriggerTimestamp(ringId * 16 + fenId, readoutTimestamp);
  }

  if (m_stats.GetMaxTriggerTimestamp(NUMFECS - 1) < readoutTimestamp) {
    m_stats.SetMaxTriggerTimestamp(NUMFECS - 1, readoutTimestamp);
  }
  if (m_stats.GetMaxTriggerTimestamp(ringId * 16 + fenId) < readoutTimestamp) {
    m_stats.SetMaxTriggerTimestamp(ringId * 16 + fenId, readoutTimestamp);
  }
  double buffer_interval_ns = 10000000.0;
  if (readoutTimestamp >= m_stats.GetOldTriggerTimestamp(ringId * 16 + fenId) +
                              buffer_interval_ns) {
    newData = true;
  }

  if (newData) {
    m_rootFile->SaveHits();
    m_stats.SetOldTriggerTimestamp(ringId * 16 + fenId, readoutTimestamp);
  }

  m_hitNr++;

  HitR5560 theHit;
  theHit.ring = ringId;
  theHit.fen = fenId;
  theHit.group = groupId;
  if (m_config.pDataFormat == 0x30) {
    theHit.counter = counter;
    theHit.ampc = ampc;
    theHit.ampd = ampd;
  } else if (m_config.pDataFormat == 0x34) {
    theHit.counter = ampc * 65536 + ampd;
    theHit.ampc = 0;
    theHit.ampd = 0;
  }
  theHit.ampa = ampa;
  theHit.ampb = ampb;

  theHit.om = om;
  theHit.time = readoutTimestamp;
  theHit.pulse_time = pulseTime;
  m_rootFile->AddHits(std::move(theHit));

  return true;
}

bool Clusterer::AnalyzeHits(double readoutTimestamp, uint8_t fecId,
                            uint8_t vmmId, uint16_t chNo, uint16_t bcid,
                            uint16_t tdc, uint16_t adc, bool overThresholdFlag,
                            double chipTime, uint8_t geoId, double pulseTime) {

  if (pulseTime > m_pulseTime[0]) {

    m_pulseTime[2] = m_pulseTime[1];
    m_pulseTime[1] = m_pulseTime[0];
    m_pulseTime[0] = pulseTime;
  }

  int pos = m_config.pPositions[fecId][vmmId][chNo];

  if (pos == -1) {
    DTRACE(DEB, "\t\tDetector or Plane not defined for FEC %d and vmmId %d!\n",
           (int)fecId, (int)vmmId);
    return true;
  }

  bool newData = false;
  double buffer_interval_ns = 10000000.0;
  if (readoutTimestamp >=
      m_stats.GetOldTriggerTimestamp(fecId) + buffer_interval_ns) {
    newData = true;
    /*std::cout << "new data: hit " << m_hitNr << ", readoutTime "
              << readoutTimestamp << ", m_stats.GetOldTriggerTimestamp("
              << (int)fecId << "): " << m_stats.GetOldTriggerTimestamp(fecId)
              << std::endl;*/
  }

  if (newData) {
    if (m_config.pSaveWhat % 2 == 1) {
      m_rootFile->SaveHits();
    }

    if (m_config.pSaveWhat >= 10) {
      uint64_t ts = 0;
      for (auto const &fec : m_config.pFecs) {
        if (fec != NUMFECS - 1) {
          if (ts == 0 || ts > m_stats.GetOldTriggerTimestamp(fec)) {
            ts = m_stats.GetOldTriggerTimestamp(fec);
          }
        }
      }

      for (auto const &det : m_config.pDets) {
        auto dp0 = std::make_pair(det.first, 0);
        auto dp1 = std::make_pair(det.first, 1);
        if (m_stats.GetLowestCommonTriggerTimestampDet(det.first) < ts) {
          m_stats.SetLowestCommonTriggerTimestampPlane(dp0, ts);
          m_stats.SetLowestCommonTriggerTimestampPlane(dp1, ts);
          m_stats.SetLowestCommonTriggerTimestampDet(det.first, ts);
          AnalyzeClustersPlane(dp0);
          AnalyzeClustersPlane(dp1);
          AnalyzeClustersDetector(det.first);
        }
      }
    }
    m_stats.SetOldTriggerTimestamp(fecId, readoutTimestamp);
  }

  m_hitNr++;
  // ESS
  // readoutTimestamp: complete including the BCID
  // chiptime:  TDC contribution and time calibration correction
  double totalTime = readoutTimestamp + chipTime;

  auto det = m_config.pDetectors[fecId][vmmId];
  auto plane = m_config.pPlanes[fecId][vmmId];

  double tof = 0;
  double jitter = 0;
  double thePulseTime = m_pulseTime[0];
  tof = totalTime - m_pulseTime[0];

  //  There could be negative TOFs, accept a jitter of up to 100ns
  if (tof < jitter && m_pulseTime[1] > 0) {
    tof = totalTime - m_pulseTime[1];
    thePulseTime = m_pulseTime[1];
    if (tof < jitter && m_pulseTime[2] > 0) {
      std::cout << "Pre-previous pulseTime:" << std::setw(19) << std::fixed
                << m_pulseTime[1] << " " << tof << std::endl;
      tof = totalTime - m_pulseTime[2];
      thePulseTime = m_pulseTime[2];
    }
    if (tof < jitter) {
      std::cout.precision(20);
      std::cout << "no pulse time found, negative tof [ns]: " << tof
                << std::endl;
      return true;
    }
  }

  double bunchIntensity = m_config.pMapPulsetimeIntensity[thePulseTime];

  if (m_config.pSaveWhat % 2 == 1) {
    if (std::find(m_config.pSaveHits.begin(), m_config.pSaveHits.end(), det) !=
        m_config.pSaveHits.end()) {

      Hit theHit;
      theHit.det = det;
      theHit.plane = plane;
      theHit.fec = fecId;
      theHit.vmm = vmmId;
      theHit.geo_id = geoId;
      theHit.pulse_time = thePulseTime;
      theHit.bunch_intensity = bunchIntensity;
      theHit.ch = chNo;
      theHit.pos = (uint16_t)pos;
      theHit.bcid = bcid;
      theHit.tdc = tdc;
      theHit.adc = adc;
      theHit.over_threshold = overThresholdFlag;
      theHit.chip_time = chipTime;
      theHit.time = totalTime;
      if (theHit.bunch_intensity >= 1E+11 && theHit.bunch_intensity <= 1E+12) {
        m_rootFile->AddHits(std::move(theHit));
      }
    }
  }
  if (overThresholdFlag) {
    // keep the overThresholdFlag as bit 15 of the ADC
    adc = adc + 32768;
  }
  if (m_config.pADCThreshold[det] < 0) {
    if (overThresholdFlag) {
      m_hits_new[std::make_pair(det, plane)].emplace_back(
          totalTime, (uint16_t)pos, adc, thePulseTime);
    }
  } else {
    if ((adc >= m_config.pADCThreshold[det])) {
      m_hits_new[std::make_pair(det, plane)].emplace_back(
          totalTime, (uint16_t)pos, adc, thePulseTime);
    }
  }

  if (m_oldFecId != fecId || newData) {
    DTRACE(DEB, "\tfecId  %d\n", fecId);
  }
  if (m_oldVmmId != vmmId || newData) {
    DTRACE(DEB, "\tDetector %d, plane %d, vmmId  %d\n", (int)det, (int)plane,
           vmmId);
  }
  DTRACE(DEB, "\t\tpos %d (chNo  %d) - overThresholdFlag %d\n", pos, chNo,
         (int)overThresholdFlag);
  DTRACE(DEB, "\t\t\tbcid %d, tdc %d, adc %d\n", bcid, tdc, adc & 0x3FF);
  DTRACE(DEB, "\t\t\ttotal time %f, chip time %f ns\n", totalTime, chipTime);

  if (m_stats.GetFirstTriggerTimestamp(fecId) == 0) {
    m_stats.SetFirstTriggerTimestamp(fecId, readoutTimestamp);
  }

  if (m_stats.GetMaxTriggerTimestamp(fecId) < readoutTimestamp) {
    m_stats.SetMaxTriggerTimestamp(fecId, readoutTimestamp);
  }
  m_oldVmmId = vmmId;
  m_oldFecId = fecId;

  return true;
}

//====================================================================================================================
int Clusterer::ClusterByTime(std::pair<uint8_t, uint8_t> dp) {

  ClusterContainer cluster;
  double maxDeltaTime = 0;
  int clusterCount = 0;
  double time1 = 0, time2 = 0;
  uint32_t adc1 = 0;
  uint16_t strip1 = 0;
  double pulseTime1 = 0;
  for (auto &itHits : m_hits[dp]) {
    time2 = time1;

    time1 = (double)std::get<0>(itHits);
    strip1 = std::get<1>(itHits);
    adc1 = std::get<2>(itHits);
    pulseTime1 = std::get<3>(itHits);
    if (!cluster.empty()) {
      if (std::fabs(time1 - time2) >
          m_config.pDeltaTimeHits[m_config.pDets[dp.first]]) {

        clusterCount += ClusterByStrip(dp, cluster, maxDeltaTime);
        cluster.clear();
        maxDeltaTime = 0.0;
      } else {
        if (maxDeltaTime < std::fabs(time1 - time2)) {
          maxDeltaTime = (time1 - time2);
        }
      }
    }
    cluster.emplace_back(strip1, time1, adc1, pulseTime1);
  }

  if (!cluster.empty()) {
    clusterCount += ClusterByStrip(dp, cluster, maxDeltaTime);
  }
  return clusterCount;

  return -1;
}

//====================================================================================================================
int Clusterer::ClusterByStrip(std::pair<uint8_t, uint8_t> dp,
                              ClusterContainer &cluster, double maxDeltaTime) {

  int maxMissingStrip = 0;
  double spanCluster = 0;

  double startTime = 0;
  double largestTime = 0;
  double largestADCTime = 0;
  double largestADCPos = 0;
  double centerOfGravity = 0;
  double centerOfTime = 0;
  double centerOfGravity2 = 0;
  double centerOfTime2 = 0;
  double centerOfGravity_ovTh = 0;
  double centerOfTime_ovTh = 0;
  double centerOfGravity2_ovTh = 0;
  double centerOfTime2_ovTh = 0;
  long int totalADC = 0;
  long int totalADC2 = 0;
  long int totalADC_ovTh = 0;
  long int totalADC2_ovTh = 0;

  double time1 = 0;
  int idx_left = 0;
  int idx_right = 0;
  double pulseTime = 0;
  int adc1 = 0;
  int adc2 = 0;
  bool ovTh = false;
  int strip1 = 0;
  int strip2 = 0;
  int stripCount = 0;
  int clusterCount = 0;
  std::vector<double> vADC;
  std::vector<double> vStrips;
  std::vector<double> vTimes;
  auto det = std::get<0>(dp);
  auto plane = std::get<1>(dp);

  std::sort(begin(cluster), end(cluster),
            [](const ClusterTuple &t1, const ClusterTuple &t2) {
              return std::get<0>(t1) < std::get<0>(t2) ||
                     (std::get<0>(t1) == std::get<0>(t2) &&
                      std::get<1>(t1) > std::get<1>(t2));
            });
  for (auto &itCluster : cluster) {
    adc2 = adc1;
    strip2 = strip1;
    strip1 = std::get<0>(itCluster);
    time1 = std::get<1>(itCluster);
    adc1 = std::get<2>(itCluster);
    pulseTime = std::get<3>(itCluster);
    if (adc1 > 1024) {
      ovTh = true;
    } else {
      ovTh = false;
    }
    adc1 = adc1 & 0x3FF;

    // At beginning of cluster, set start time of cluster
    if (stripCount == 0) {
      maxMissingStrip = 0;
      idx_left = 0;
      idx_right = 0;
      startTime = time1;
      largestTime = time1;
      largestADCTime = time1;
      largestADCPos = strip1;
      DTRACE(DEB, "\nDetector %d, plane %d cluster:\n", (int)det, (int)plane);
    }

    // Add members of a cluster, if it is either the beginning of a cluster,
    // or if strip gap and time span is correct
    if (stripCount == 0 ||
        (((plane == 2 && m_config.pAlgo == 5) ||
          (std::fabs(strip1 - strip2) - 1 <=
           m_config.pMissingStripsCluster[m_config.pDets[dp.first]])) &&
         time1 - startTime <=
             m_config.pSpanClusterTime[m_config.pDets[dp.first]] &&
         largestTime - time1 <=
             m_config.pSpanClusterTime[m_config.pDets[dp.first]])) {
      DTRACE(DEB, "\tstrip %d, time %llu, adc %d:\n", strip1,
             static_cast<unsigned long long>(time1), adc1);

      if (adc1 > adc2) {
        largestADCTime = time1;
        largestADCPos = strip1;
      }

      if (time1 == largestTime) {
        idx_right = stripCount;
      }
      if (time1 > largestTime) {
        idx_left = stripCount;
        idx_right = stripCount;
        largestTime = time1;
      }
      if (time1 < startTime) {
        startTime = time1;
      }
      if (stripCount > 0 && maxMissingStrip < std::fabs(strip1 - strip2) - 1) {
        maxMissingStrip = std::fabs(strip1 - strip2) - 1;
      }
      spanCluster = (largestTime - startTime);
      totalADC += adc1;
      totalADC2 += adc1 * adc1;
      centerOfGravity += strip1 * adc1;
      centerOfTime += time1 * adc1;
      centerOfGravity2 += strip1 * adc1 * adc1;
      centerOfTime2 += time1 * adc1 * adc1;

      if (ovTh) {
        totalADC_ovTh += adc1;
        totalADC2_ovTh += adc1 * adc1;
        centerOfGravity_ovTh += strip1 * adc1;
        centerOfTime_ovTh += time1 * adc1;
        centerOfGravity2_ovTh += strip1 * adc1 * adc1;
        centerOfTime2_ovTh += time1 * adc1 * adc1;
      }

      vStrips.emplace_back(strip1);
      vTimes.emplace_back(time1);
      vADC.emplace_back(adc1);
      stripCount++;
    }
    // Stop clustering if gap between strips is too large or time span too
    // long
    else if ((!(plane == 2 && m_config.pAlgo == 5) &&
              (std::fabs(strip1 - strip2) - 1 >
               m_config.pMissingStripsCluster[m_config.pDets[dp.first]])) ||
             time1 - startTime >
                 m_config.pSpanClusterTime[m_config.pDets[dp.first]] ||
             largestTime - time1 >
                 m_config.pSpanClusterTime[m_config.pDets[dp.first]]) {
      // Valid cluster
      if (stripCount < m_config.pMinClusterSize[m_config.pDets[dp.first]] ||
          totalADC == 0) {
        DTRACE(DEB, "******** INVALID CLUSTER SIZE ********%d\n\n", stripCount);
      } else {

        spanCluster = (largestTime - startTime);
        centerOfGravity = (centerOfGravity / totalADC);
        centerOfTime = (centerOfTime / totalADC);
        centerOfGravity2 = (centerOfGravity2 / totalADC2);
        centerOfTime2 = (centerOfTime2 / totalADC2);

        if (totalADC_ovTh > 0) {
          centerOfGravity_ovTh = (centerOfGravity_ovTh / totalADC_ovTh);
          centerOfTime_ovTh = (centerOfTime_ovTh / totalADC_ovTh);
        }
        if (totalADC2_ovTh > 0) {
          centerOfGravity2_ovTh = (centerOfGravity2_ovTh / totalADC2_ovTh);
          centerOfTime2_ovTh = (centerOfTime2_ovTh / totalADC2_ovTh);
        }
        if (m_config.pShowStats) {
          m_stats.SetStatsPlane("DeltaTimeHits", dp, maxDeltaTime);
          m_stats.SetStatsPlane("MissingStripsCluster", dp, maxMissingStrip);
          m_stats.SetStatsPlane("SpanClusterTime", dp, spanCluster);
          m_stats.SetStatsPlane("ClusterSize", dp, stripCount);
        }

        ClusterPlane clusterPlane;
        clusterPlane.pulse_time = pulseTime;
        clusterPlane.bunch_intensity =
            m_config.pMapPulsetimeIntensity[pulseTime];
        clusterPlane.size = stripCount;
        clusterPlane.adc = totalADC;
        clusterPlane.time = centerOfTime;
        clusterPlane.pos = centerOfGravity;

        clusterPlane.time_charge2 = centerOfTime2;
        clusterPlane.pos_charge2 = centerOfGravity2;

        double time_utpc = 0;
        double pos_utpc = 0;
        double time_algo = 0;
        double pos_algo = 0;
        AlgorithmUTPC(idx_left, idx_right, vADC, vStrips, vTimes, pos_utpc,
                      time_utpc, pos_algo, time_algo);
        // COT only over Threshold
        if (m_config.pAlgo == 2) {
          pos_algo = centerOfGravity_ovTh;
          time_algo = centerOfTime_ovTh;
        }
        // COT2 only over Threshold
        else if (m_config.pAlgo == 3) {
          pos_algo = centerOfGravity2_ovTh;
          time_algo = centerOfTime2_ovTh;
        }
        // time of highest ADC
        else if (m_config.pAlgo == 4) {
          pos_algo = largestADCPos;
          time_algo = largestADCTime;
        }

        clusterPlane.time_utpc = time_utpc;
        clusterPlane.pos_utpc = pos_utpc;
        clusterPlane.time_algo = time_algo;
        clusterPlane.pos_algo = pos_algo;

        clusterPlane.plane_coincidence = false;
        clusterPlane.max_delta_time = maxDeltaTime;
        clusterPlane.max_missing_strip = maxMissingStrip;
        clusterPlane.span_cluster = spanCluster;
        clusterPlane.strips = std::move(vStrips);
        clusterPlane.times = std::move(vTimes);
        clusterPlane.adcs = std::move(vADC);

        m_cluster_id++;

        DTRACE(DEB, "Cluster id %d\n", m_cluster_id);
        clusterPlane.det = det;
        clusterPlane.plane = plane;
        if (clusterPlane.bunch_intensity >= 1E+11 &&
            clusterPlane.bunch_intensity <= 1E+12) {
          m_clusters_new[dp].emplace_back(std::move(clusterPlane));
        }
        if (m_config.pShowStats) {
          m_stats.SetStatsPlane("ClusterCntPlane", dp, 0);
        }
        clusterCount++;
      }

      // Clear vectors
      vADC.clear();
      vStrips.clear();
      vTimes.clear();
      // Strip that caused gap in cluster is added as first strip of new
      // cluster
      vStrips.emplace_back(strip1);
      vTimes.emplace_back(time1);
      vADC.emplace_back(adc1);
      stripCount = 1;
      largestADCTime = time1;
      largestADCPos = strip1;
      idx_right = 0;
      idx_left = 0;
      largestTime = time1;
      startTime = time1;
      maxMissingStrip = 0;
      spanCluster = 0;
      totalADC = adc1;
      totalADC2 = adc1 * adc1;
      centerOfGravity = strip1 * adc1;
      centerOfTime = time1 * adc1;
      centerOfGravity2 = strip1 * adc1 * adc1;
      centerOfTime2 = time1 * adc1 * adc1;

      if (ovTh) {
        totalADC_ovTh = adc1;
        totalADC2_ovTh = adc1 * adc1;
        centerOfGravity_ovTh = strip1 * adc1;
        centerOfTime_ovTh = time1 * adc1;
        centerOfGravity2_ovTh = strip1 * adc1 * adc1;
        centerOfTime2_ovTh = time1 * adc1 * adc1;
      }
    }
  }

  // At the end of the clustering, check again if there is a last valid
  // cluster
  if (stripCount >= m_config.pMinClusterSize[m_config.pDets[dp.first]] &&
      totalADC > 0) {

    spanCluster = (largestTime - startTime);
    centerOfGravity = (centerOfGravity / totalADC);
    centerOfTime = (centerOfTime / totalADC);
    centerOfGravity2 = (centerOfGravity2 / totalADC2);
    centerOfTime2 = (centerOfTime2 / totalADC2);

    // std::cout << "Type 1 cluster maxDeltaTime " << maxDeltaTime << ",
    // strip_count " << stripCount << ", spanCluster " << spanCluster << ",
    // largestTime " << largestTime << ", starttime " << startTime << std::endl;

    if (m_config.pShowStats) {
      m_stats.SetStatsPlane("DeltaTimeHits", dp, maxDeltaTime);
      m_stats.SetStatsPlane("MissingStripsCluster", dp, maxMissingStrip);
      m_stats.SetStatsPlane("SpanClusterTime", dp, spanCluster);
      m_stats.SetStatsPlane("ClusterSize", dp, stripCount);
    }
    ClusterPlane clusterPlane;
    clusterPlane.pulse_time = pulseTime;
    clusterPlane.bunch_intensity = m_config.pMapPulsetimeIntensity[pulseTime];
    clusterPlane.size = stripCount;
    clusterPlane.adc = totalADC;
    clusterPlane.time = centerOfTime;
    clusterPlane.pos = centerOfGravity;
    clusterPlane.time_charge2 = centerOfTime2;
    clusterPlane.pos_charge2 = centerOfGravity2;

    double time_utpc = 0;
    double pos_utpc = 0;
    double time_algo = 0;
    double pos_algo = 0;
    AlgorithmUTPC(idx_left, idx_right, vADC, vStrips, vTimes, pos_utpc,
                  time_utpc, pos_algo, time_algo);

    // COT only over Threshold
    if (m_config.pAlgo == 2) {
      pos_algo = centerOfGravity_ovTh;
      time_algo = centerOfTime_ovTh;
    }
    // COT2 only over Threshold
    else if (m_config.pAlgo == 3) {
      pos_algo = centerOfGravity2_ovTh;
      time_algo = centerOfTime2_ovTh;
    }
    // time of highest ADC
    else if (m_config.pAlgo == 4) {
      pos_algo = largestADCPos;
      time_algo = largestADCTime;
    }

    clusterPlane.time_utpc = time_utpc;
    clusterPlane.pos_utpc = pos_utpc;
    clusterPlane.time_algo = time_algo;
    clusterPlane.pos_algo = pos_algo;

    clusterPlane.plane_coincidence = false;
    clusterPlane.max_delta_time = maxDeltaTime;
    clusterPlane.max_missing_strip = maxMissingStrip;
    clusterPlane.span_cluster = spanCluster;
    clusterPlane.strips = std::move(vStrips);
    clusterPlane.times = std::move(vTimes);
    clusterPlane.adcs = std::move(vADC);
    m_cluster_id++;

    DTRACE(DEB, "Cluster id %d\n", m_cluster_id);

    clusterPlane.det = det;
    clusterPlane.plane = plane;
    if (clusterPlane.bunch_intensity >= 1E+11 &&
        clusterPlane.bunch_intensity <= 1E+12) {
      m_clusters_new[dp].emplace_back(std::move(clusterPlane));
    }
    if (m_config.pShowStats) {
      m_stats.SetStatsPlane("ClusterCntPlane", dp, 0);
    }
    clusterCount++;
  }

  return clusterCount;
}

void Clusterer::AlgorithmUTPC(int idx_min_largest_time,
                              int idx_max_largest_time,
                              std::vector<double> &vADC,
                              std::vector<double> &vStrips,
                              std::vector<double> &vTimes, double &positionUTPC,
                              double &timeUTPC, double &positionAlgo,
                              double &timeAlgo) {

  double a1 = 0, a2 = 0, a3 = 0, p1 = 0, p2 = 0, p3 = 0, t1 = 0, t2 = 0, t3 = 0;
  int idx_largest_time = 0;
  // One largest time exists
  if (idx_max_largest_time == idx_min_largest_time) {
    idx_largest_time = idx_max_largest_time;
    positionUTPC = vStrips[idx_largest_time];
    timeUTPC = vTimes[idx_largest_time];
  } else {
    // More than one largest time, the right most largest time strip
    // is closer to the end of the track than the lest most
    if (vStrips.size() - 1 - idx_max_largest_time < idx_min_largest_time) {
      idx_largest_time = idx_max_largest_time;
    }
    // More than one largest time, the left most largest time strip
    // is closer to the end of the track than the right most
    else if (vStrips.size() - 1 - idx_max_largest_time > idx_min_largest_time) {
      idx_largest_time = idx_min_largest_time;
    }
    // More than one largest time, the left and right most largest time strips
    // have an identical distance to the start/end of the track
    // Take the strip with the larges ADC
    else {
      if (vADC[idx_min_largest_time] > vADC[idx_max_largest_time]) {
        idx_largest_time = idx_min_largest_time;
      } else if (vADC[idx_min_largest_time] <= vADC[idx_max_largest_time]) {
        idx_largest_time = idx_max_largest_time;
      }
    }
  }

  positionUTPC = vStrips[idx_largest_time];
  timeUTPC = vTimes[idx_largest_time];

  p2 = vStrips[idx_largest_time];
  a2 = vADC[idx_largest_time];
  t2 = vTimes[idx_largest_time];
  if (idx_largest_time > 0) {
    p1 = vStrips[idx_largest_time - 1];
    a1 = vADC[idx_largest_time - 1];
    t1 = vTimes[idx_largest_time - 1];
  }
  if (idx_largest_time < vStrips.size() - 1) {
    p3 = vStrips[idx_largest_time + 1];
    a3 = vADC[idx_largest_time + 1];
    t3 = vTimes[idx_largest_time + 1];
  }
  if (m_config.pAlgo == 1) {
    positionAlgo = (p1 * a1 * a1 + p2 * a2 * a2 + p3 * a3 * a3) /
                   (a1 * a1 + a2 * a2 + a3 * a3);
    timeAlgo = (t1 * a1 * a1 + t2 * a2 * a2 + t3 * a3 * a3) /
               (a1 * a1 + a2 * a2 + a3 * a3);
  } else if (m_config.pAlgo == 0) {
    positionAlgo = (p1 * a1 + p2 * a2 + p3 * a3) / (a1 + a2 + a3);
    timeAlgo = (t1 * a1 + t2 * a2 + t3 * a3) / (a1 + a2 + a3);

  } else if (m_config.pAlgo == 6) {
    double slope = -99999.0;
    double offset = -99999.0;
    double sum_dev = 0;
    double sumX = 0, sumY = 0, sumXY = 0, sumX2 = 0;
    size_t nPoints = vStrips.size();

    auto min_time = *std::min_element(vTimes.begin(), vTimes.end());
    if (nPoints >= 2) {
      for (int n = 0; n < nPoints; n++) {
        double theMean = 0;
        double theXValue = 0;
        sumX += vStrips[n];
        sumY += vTimes[n] - min_time;
        sumXY += vStrips[n] * (vTimes[n] - min_time);
        sumX2 += vStrips[n] * vStrips[n];
      }

      double xMean = sumX / static_cast<double>(nPoints);
      double yMean = sumY / static_cast<double>(nPoints);
      double denominator = sumX2 - sumX * xMean;
      if (std::fabs(denominator) > 1e-7) {
        slope = (sumXY - sumX * yMean) / denominator;
        offset = yMean - slope * xMean;
      }
      slope = 0.009 * slope;
      offset = 0.009 * offset;

      for (int n = 0; n < nPoints; n++) {
        double y_fit = slope * vStrips[n] + offset;
        double y_meas = 0.009 * (vTimes[n] - min_time);
        double delta_y = y_fit - y_meas;
        double delta_fit = delta_y * std::cos(std::atan(slope));
        sum_dev += std::abs(delta_fit);
      }
      sum_dev = sum_dev / nPoints;
    }
    positionAlgo = slope;
    timeAlgo = offset;
    positionUTPC = sum_dev;
  }
}

//====================================================================================================================
int Clusterer::MatchClustersDetector(uint8_t det) {

  int clusterCount = 0;
  auto dp0 = std::make_pair(det, 0);
  auto dp1 = std::make_pair(det, 1);

  ClusterVectorPlane::iterator itStartPlane1 = begin(m_clusters[dp1]);

  for (auto &c0 : m_clusters[dp0]) {
    double minDelta1 = std::numeric_limits<double>::max();
    double lastDelta_t1 = std::numeric_limits<double>::max();
    double delta_t1 = std::numeric_limits<double>::max();
    bool isFirstMatch1 = true;
    ClusterVectorPlane::iterator bestMatchPlane1 = end(m_clusters[dp1]);
    for (ClusterVectorPlane::iterator c1 = itStartPlane1;
         c1 != end(m_clusters[dp1]); ++c1) {
      if ((*c1).plane_coincidence == false) {
        double chargeRatio1 = (double)(c0).adc / (double)(*c1).adc;
        lastDelta_t1 = delta_t1;
        delta_t1 = (*c1).time - c0.time;
        if (m_config.pConditionCoincidence == "utpc") {
          delta_t1 = (*c1).time_utpc - c0.time_utpc;
        } else if (m_config.pConditionCoincidence == "charge2") {
          delta_t1 = (*c1).time_charge2 - c0.time_charge2;
        }
        if (std::fabs(delta_t1) <=
            m_config.pDeltaTimePlanes[m_config.pDets[det]]) {
          if (isFirstMatch1) {
            itStartPlane1 = c1;
            isFirstMatch1 = false;
          }
        }

        if (chargeRatio1 >= m_config.pChargeRatioLower[m_config.pDets[det]] &&
            chargeRatio1 <= m_config.pChargeRatioUpper[m_config.pDets[det]] &&
            std::fabs(delta_t1) < minDelta1 &&
            std::fabs(delta_t1) <=
                m_config.pDeltaTimePlanes[m_config.pDets[det]] &&
            (c0.size + (*c1).size >=
             m_config.pCoincidentClusterSize[m_config.pDets[det]])) {
          minDelta1 = std::fabs(delta_t1);
          bestMatchPlane1 = c1;
        }
        if (std::fabs(delta_t1) > std::fabs(lastDelta_t1) &&
            std::fabs(delta_t1) >
                m_config.pDeltaTimePlanes[m_config.pDets[det]]) {
          break;
        }
      }
    }

    if (bestMatchPlane1 != end(m_clusters[dp1])) {
      m_cluster_detector_id++;
      ClusterDetector clusterDetector;
      c0.plane_coincidence = true;
      (*bestMatchPlane1).plane_coincidence = true;
      clusterDetector.det = det;
      clusterDetector.size0 = c0.size;
      clusterDetector.size1 = (*bestMatchPlane1).size;
      clusterDetector.adc0 = c0.adc;
      clusterDetector.adc1 = (*bestMatchPlane1).adc;
      clusterDetector.pos0 = c0.pos;
      clusterDetector.pos1 = (*bestMatchPlane1).pos;
      clusterDetector.pos0_utpc = c0.pos_utpc;
      clusterDetector.pos1_utpc = (*bestMatchPlane1).pos_utpc;
      clusterDetector.pos0_charge2 = c0.pos_charge2;
      clusterDetector.pos1_charge2 = (*bestMatchPlane1).pos_charge2;
      clusterDetector.pos0_algo = c0.pos_algo;
      clusterDetector.pos1_algo = (*bestMatchPlane1).pos_algo;
      clusterDetector.pulse_time =
          (c0.pulse_time + (*bestMatchPlane1).pulse_time) / 2;
      clusterDetector.bunch_intensity =
          (c0.bunch_intensity + (*bestMatchPlane1).bunch_intensity) / 2;
      clusterDetector.time0 = c0.time;
      clusterDetector.time0_utpc = c0.time_utpc;
      clusterDetector.time0_charge2 = c0.time_charge2;
      clusterDetector.time0_algo = c0.time_algo;
      clusterDetector.time1 = (*bestMatchPlane1).time;
      clusterDetector.time1_utpc = (*bestMatchPlane1).time_utpc;
      clusterDetector.time1_charge2 = (*bestMatchPlane1).time_charge2;
      clusterDetector.time1_algo = (*bestMatchPlane1).time_algo;
      clusterDetector.dt0 = c0.time - last_time0;
      clusterDetector.dt1 = (*bestMatchPlane1).time - last_time1;
      clusterDetector.max_delta_time0 = c0.max_delta_time;
      clusterDetector.max_delta_time1 = (*bestMatchPlane1).max_delta_time;
      clusterDetector.max_missing_strip0 = c0.max_missing_strip;
      clusterDetector.max_missing_strip1 = (*bestMatchPlane1).max_missing_strip;
      clusterDetector.span_cluster0 = c0.span_cluster;
      clusterDetector.span_cluster1 = (*bestMatchPlane1).span_cluster;
      clusterDetector.strips0 = c0.strips;
      clusterDetector.times0 = c0.times;
      clusterDetector.adcs0 = c0.adcs;
      clusterDetector.strips1 = (*bestMatchPlane1).strips;
      clusterDetector.times1 = (*bestMatchPlane1).times;
      clusterDetector.adcs1 = (*bestMatchPlane1).adcs;

      last_time0 = clusterDetector.time0;
      last_time1 = clusterDetector.time1;

      clusterDetector.delta_plane_0_1 =
          clusterDetector.time1 - clusterDetector.time0;

      if (m_config.pConditionCoincidence == "utpc") {
        clusterDetector.delta_plane_0_1 =
            clusterDetector.time1_utpc - clusterDetector.time0_utpc;
      } else if (m_config.pConditionCoincidence == "charge2") {
        clusterDetector.delta_plane_0_1 =
            clusterDetector.time1_charge2 - clusterDetector.time0_charge2;
      }

      if (m_config.pShowStats) {
        m_stats.SetStatsDetector("DeltaTimePlanes_0_1", det,
                                 std::fabs(clusterDetector.delta_plane_0_1));

        double ratio =
            100 * (double)clusterDetector.adc0 / (double)clusterDetector.adc1;
        if (ratio > 100.0) {
          ratio =
              100 * (double)clusterDetector.adc1 / (double)clusterDetector.adc0;
          m_stats.SetStatsDetector("ChargeRatio_1_0", det, ratio);
        } else {
          m_stats.SetStatsDetector("ChargeRatio_0_1", det, ratio);
        }

        m_stats.SetStatsDetector("ClusterCntDetector", det, 0);
        clusterCount++;
      }
      DTRACE(DEB, "\ncommon cluster det %d", (int)det);
      DTRACE(DEB, "\tpos x/pos y: %f/%f", clusterDetector.pos0,
             clusterDetector.pos1);
      DTRACE(DEB, "\ttime x/time y: %llu/%llu",
             static_cast<unsigned long long>(clusterDetector.time0),
             static_cast<unsigned long long>(clusterDetector.time1));
      DTRACE(DEB, "\tadc x/adc y: %u/%u", clusterDetector.adc0,
             clusterDetector.adc1);
      DTRACE(DEB, "\tsize x/size y: %u/%u", clusterDetector.size0,
             clusterDetector.size1);
      DTRACE(DEB, "\tdelta time planes: %d\n",
             (int)clusterDetector.delta_plane_0_1);

      if (clusterDetector.bunch_intensity >= 1E+11 &&
          clusterDetector.bunch_intensity <= 1E+12) {
        m_clusters_detector[det].emplace_back(std::move(clusterDetector));
      }
    }
  }

  return clusterCount;
}

//====================================================================================================================

void Clusterer::AnalyzeClustersPlane(std::pair<uint8_t, uint8_t> dp) {
  if (ChooseHitsToBeClustered(dp) == false && m_hits[dp].empty()) {
    return;
  }
  int cnt = ClusterByTime(dp);
  DTRACE(DEB, "%d cluster in detector %d plane %d\n", cnt, (int)std::get<0>(dp),
         (int)std::get<1>(dp));

  m_hits[dp].clear();
}

void Clusterer::AnalyzeClustersDetector(uint8_t det) {
  int cnt = 0;
  auto dp0 = std::make_pair(det, 0);
  auto dp1 = std::make_pair(det, 1);

  if (ChooseClustersToBeMatched(dp0) == false && m_clusters[dp0].empty()) {
    return;
  }
  if (ChooseClustersToBeMatched(dp1) == false && m_clusters[dp1].empty()) {
    return;
  }

  cnt = MatchClustersDetector(det);

  if (m_config.pSaveWhat == 10 || m_config.pSaveWhat == 11 ||
      m_config.pSaveWhat == 110 || m_config.pSaveWhat == 111) {
    m_rootFile->SaveClustersPlane(std::move(m_clusters[dp0]));
    m_rootFile->SaveClustersPlane(std::move(m_clusters[dp1]));
  }

  if (m_config.pSaveWhat >= 100) {
    m_rootFile->SaveClustersDetector(std::move(m_clusters_detector[det]));
  }
  m_clusters[std::make_pair(det, 0)].clear();
  m_clusters[std::make_pair(det, 1)].clear();
  m_clusters_detector[det].clear();
}

bool Clusterer::ChooseHitsToBeClustered(std::pair<uint8_t, uint8_t> dp) {

  // std::pair<uint8_t, uint8_t> dp = std::make_pair(det, plane);
  double timeReadyToCluster = m_stats.GetLowestCommonTriggerTimestampPlane(dp);
  // Nothing to cluster, newHits vector empty
  if (m_hits_new[dp].empty()) {
    return false;
  }

  auto theMin = std::min_element(m_hits_new[dp].begin(), m_hits_new[dp].end(),
                                 [](const HitTuple &t1, const HitTuple &t2) {
                                   return std::get<0>(t1) < std::get<0>(t2);
                                 });

  // Nothing to cluster, tuples in newHits vector too recent
  if (std::get<0>(*theMin) > timeReadyToCluster) {
    //(smallest timestamp larger than
    // m_stats.GetLowestCommonTriggerTimestampPlane(dp)) Will be clustered
    // later
    return false;
  }

  // Sort vector newHits
  std::sort(begin(m_hits_new[dp]), end(m_hits_new[dp]),
            [](const HitTuple &t1, const HitTuple &t2) {
              return std::get<0>(t1) < std::get<0>(t2);
            });

  // First tuple with timestamp larger than
  // m_stats.GetLowestCommonTriggerTimestampPlane(dp)
  auto it = std::upper_bound(
      m_hits_new[dp].begin(), m_hits_new[dp].end(),
      std::make_tuple(m_stats.GetLowestCommonTriggerTimestampPlane(dp), 0, 0,
                      0),
      [](const HitTuple &t1, const HitTuple &t2) {
        return std::get<0>(t1) < std::get<0>(t2);
      });

  // Find elements in vector that could still be part of a cluster,
  // since they are close in time to
  // m_stats.GetLowestCommonTriggerTimestampPlane(dp)
  while (it != m_hits_new[dp].end()) {
    if (std::get<0>(*it) - timeReadyToCluster >
        m_config.pDeltaTimeHits[m_config.pDets[dp.first]]) {
      break;
    }
    timeReadyToCluster = std::get<0>(*it);
    ++it;
  }
  int index = std::distance(m_hits_new[dp].begin(), it);
  // Insert the data that is ready to be clustered from newHits into hits
  m_hits[dp].insert(m_hits[dp].end(),
                    std::make_move_iterator(m_hits_new[dp].begin()),
                    std::make_move_iterator(m_hits_new[dp].begin() + index));
  // Delete the data from newHits
  m_hits_new[dp].erase(m_hits_new[dp].begin(), m_hits_new[dp].begin() + index);

  return true;
}

bool Clusterer::ChooseClustersToBeMatched(std::pair<uint8_t, uint8_t> dp) {
  int index = 0;
  // std::pair<uint8_t, uint8_t> dp = std::make_pair(det, plane);
  double timeReadyToMatch = m_stats.GetLowestCommonTriggerTimestampPlane(dp);

  // Nothing to match, newClusters vector empty
  if (m_clusters_new[dp].empty()) {
    return false;
  }
  if (m_config.pConditionCoincidence == "utpc") {
    auto theMin =
        std::min_element(m_clusters_new[dp].begin(), m_clusters_new[dp].end(),
                         [](const ClusterPlane &t1, const ClusterPlane &t2) {
                           return t1.time_utpc < t2.time_utpc;
                         });

    // Nothing to cluster, clusters in newClusters vector too recent
    if ((*theMin).time_utpc > timeReadyToMatch) {

      //(smallest time larger than timeReadyToMatch)
      // Will be matched later
      return false;
    }

    // Sort vector newClusters based on time
    std::sort(begin(m_clusters_new[dp]), end(m_clusters_new[dp]),
              [](const ClusterPlane &t1, const ClusterPlane &t2) {
                return t1.time_utpc < t2.time_utpc;
              });

    ClusterPlane theCluster;
    theCluster.time_utpc = timeReadyToMatch;

    // First ClusterPlane with time bigger than timeReadyToMatch
    auto it = std::upper_bound(
        m_clusters_new[dp].begin(), m_clusters_new[dp].end(), theCluster,
        [](const ClusterPlane &t1, const ClusterPlane &t2) {
          return t1.time_utpc < t2.time_utpc;
        });

    // Find elements in vector that could still be matched with another
    // cluster since they are close in time to timeReadyToMatch
    while (it != m_clusters_new[dp].end()) {
      if ((*it).time_utpc - timeReadyToMatch >
          m_config.pDeltaTimeHits[m_config.pDets[dp.first]]) {
        break;
      }
      timeReadyToMatch = (*it).time_utpc;
      ++it;
    }
    index = std::distance(m_clusters_new[dp].begin(), it);
  } else if (m_config.pConditionCoincidence == "charge2") {
    auto theMin =
        std::min_element(m_clusters_new[dp].begin(), m_clusters_new[dp].end(),
                         [](const ClusterPlane &t1, const ClusterPlane &t2) {
                           return t1.time_charge2 < t2.time_charge2;
                         });

    // Nothing to cluster, clusters in newClusters vector too recent
    if ((*theMin).time_charge2 > timeReadyToMatch) {

      //(smallest time larger than timeReadyToMatch)
      // Will be matched later
      return false;
    }

    // Sort vector newClusters based on time
    std::sort(begin(m_clusters_new[dp]), end(m_clusters_new[dp]),
              [](const ClusterPlane &t1, const ClusterPlane &t2) {
                return t1.time_charge2 < t2.time_charge2;
              });

    ClusterPlane theCluster;
    theCluster.time_charge2 = timeReadyToMatch;

    // First ClusterPlane with time that bigger than timeReadyToMatch
    auto it = std::upper_bound(
        m_clusters_new[dp].begin(), m_clusters_new[dp].end(), theCluster,
        [](const ClusterPlane &t1, const ClusterPlane &t2) {
          return t1.time_charge2 < t2.time_charge2;
        });

    // Find elements in vector that could still be matched with another
    // cluster since they are close in time to timeReadyToMatch
    while (it != m_clusters_new[dp].end()) {
      if ((*it).time_charge2 - timeReadyToMatch >
          m_config.pDeltaTimeHits[m_config.pDets[dp.first]]) {
        break;
      }
      timeReadyToMatch = (*it).time_charge2;
      ++it;
    }
    index = std::distance(m_clusters_new[dp].begin(), it);
  } else {
    auto theMin =
        std::min_element(m_clusters_new[dp].begin(), m_clusters_new[dp].end(),
                         [](const ClusterPlane &t1, const ClusterPlane &t2) {
                           return t1.time < t2.time;
                         });

    // Nothing to cluster, clusters in newClusters vector too recent
    if ((*theMin).time > timeReadyToMatch) {
      //(smallest time larger than timeReadyToMatch)
      // Will be matched later
      return false;
    }

    // Sort vector newClusters based on time
    std::sort(begin(m_clusters_new[dp]), end(m_clusters_new[dp]),
              [](const ClusterPlane &t1, const ClusterPlane &t2) {
                return t1.time < t2.time;
              });

    ClusterPlane theCluster;
    theCluster.time = timeReadyToMatch;

    // First ClusterPlane with time that bigger than timeReadyToMatch
    auto it = std::upper_bound(
        m_clusters_new[dp].begin(), m_clusters_new[dp].end(), theCluster,
        [](const ClusterPlane &t1, const ClusterPlane &t2) {
          return t1.time < t2.time;
        });

    // Find elements in vector that could still be matched with another
    // cluster since they are close in time to timeReadyToMatch
    while (it != m_clusters_new[dp].end()) {
      if ((*it).time - timeReadyToMatch >
          m_config.pDeltaTimeHits[m_config.pDets[dp.first]]) {
        break;
      }
      timeReadyToMatch = (*it).time;
      ++it;
    }
    index = std::distance(m_clusters_new[dp].begin(), it);
  }
  // Insert the clusters that are ready to be matched from newClusters into
  // clusters
  m_clusters[dp].insert(
      m_clusters[dp].end(), std::make_move_iterator(m_clusters_new[dp].begin()),
      std::make_move_iterator(m_clusters_new[dp].begin() + index));
  // Delete the clusters from newClusters
  m_clusters_new[dp].erase(m_clusters_new[dp].begin(),
                           m_clusters_new[dp].begin() + index);

  return true;
}

void Clusterer::FinishAnalysis() {
  double ts = 0;
  for (auto const &fec : m_config.pFecs) {
    if (ts < m_stats.GetMaxTriggerTimestamp(fec)) {
      ts = m_stats.GetMaxTriggerTimestamp(fec);
    }
  }
  for (auto const &det : m_config.pDets) {
    auto dp0 = std::make_pair(det.first, 0);
    auto dp1 = std::make_pair(det.first, 1);

    // Set the largest timestamp of plane to detector
    // cluster all remaining data in plane
    m_stats.SetLowestCommonTriggerTimestampPlane(dp0, ts);
    m_stats.SetLowestCommonTriggerTimestampPlane(dp1, ts);
    m_stats.SetLowestCommonTriggerTimestampDet(det.first, ts);

    AnalyzeClustersPlane(dp0);
    AnalyzeClustersPlane(dp1);

    AnalyzeClustersDetector(det.first);

    if (m_config.pSaveWhat % 2 == 1) {
      m_rootFile->SaveHits();
    }
  }
  if (m_config.pShowStats) {
    if (m_config.pSaveWhat >= 10) {
      m_stats.PrintClusterStats(m_config);
    }
    m_stats.PrintFECStats(m_config);
  }
}

void Clusterer::SaveDate(double the_seconds_start, std::string the_date_start,
                         double the_seconds_end, std::string the_date_end,
                         uint64_t num_triggers) {
  std::cout << "\nXXXXXXXXXXXXXXXXXXXXXXXXXXX Date and time of first pcapng "
               "packet XXXXXXXXXXXXXXXXXXXXXXXXXXX"
            << std::endl;
  std::cout << the_date_start << std::endl;
  m_rootFile->SaveDate(the_seconds_start, the_date_start, the_seconds_end,
                       the_date_end, num_triggers);
}

void Clusterer::FillCalibHistos(uint16_t fec, uint8_t vmm, uint8_t ch,
                                float adc, float adc_corrected, float chip_time,
                                float chip_time_corrected) {
  if (m_config.useCalibration && m_config.calibrationHistogram) {
    m_rootFile->FillCalibHistos(fec, vmm, ch, adc, adc_corrected, chip_time,
                                chip_time_corrected);
  }
}