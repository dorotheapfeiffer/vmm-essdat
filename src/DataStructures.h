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
**  DataStructures.h
**
****************************************************************************/

#pragma once
#include <vector>

struct Hit {
  uint8_t det;
  uint8_t plane;
  uint8_t fec;
  uint8_t vmm;
  int64_t pulse_time;
  double bunch_intensity;
  int64_t time;
  uint8_t geo_id;
  uint8_t ch;
  uint16_t pos;
  uint16_t bcid;
  uint16_t tdc;
  uint16_t adc;
  bool over_threshold;
  int64_t chip_time;
};

struct HitR5560 {
  uint8_t ring;
  uint8_t fen;
  uint8_t group;
  uint32_t counter;
  uint16_t ampa;
  uint16_t ampb;
  uint16_t ampc;
  uint16_t ampd;
  uint8_t om;
  int64_t pulse_time;
  int64_t time;
};

struct HitIBM {
  uint8_t ring;
  uint8_t fen;
  uint8_t type;
  uint32_t adc;
  uint16_t samples;
  int64_t pulse_time;
  int64_t time;
};

struct HitCDT {
  uint8_t ring;
  uint8_t fen;
  uint8_t om;
  uint8_t uid;
  uint8_t cathode;
  uint8_t anode;
  int64_t pulse_time;
  int64_t time;
};

struct ClusterPlane {
  uint8_t det;
  uint8_t plane;
  int64_t pulse_time;
  double bunch_intensity;
  uint16_t size;
  uint32_t adc;
  int64_t time;
  int64_t time_utpc;
  int64_t time_charge2;
  int64_t time_algo;
  double pos;
  double pos_utpc;
  double pos_charge2;
  double pos_algo;
  bool plane_coincidence;
  uint16_t max_delta_time;
  uint16_t max_missing_strip;
  uint16_t span_cluster;
  std::vector<uint16_t> strips;
  std::vector<int64_t> times;
  std::vector<uint16_t> adcs;
};

struct ClusterDetector {
  uint8_t det;
  int64_t pulse_time;
  double bunch_intensity;
  uint16_t size0;
  uint16_t size1;
  uint32_t adc0;
  uint32_t adc1;
  double pos0;
  double pos1;
  int64_t time0;
  int64_t time1;
  double pos0_utpc;
  double pos1_utpc;
  int64_t time0_utpc;
  int64_t time1_utpc;
  double pos0_charge2;
  double pos1_charge2;
  int64_t time0_charge2;
  int64_t time1_charge2;
  double pos0_algo;
  double pos1_algo;
  int64_t time0_algo;
  int64_t time1_algo;
  int64_t dt0;
  int64_t dt1;
  int64_t delta_plane_0_1;
  uint16_t span_cluster0;
  uint16_t span_cluster1;
  uint16_t max_delta_time0;
  uint16_t max_delta_time1;
  uint16_t max_missing_strip0;
  uint16_t max_missing_strip1;
  std::vector<uint16_t> strips0;
  std::vector<int64_t> times0;
  std::vector<uint16_t> adcs0;
  std::vector<uint16_t> strips1;
  std::vector<int64_t> times1;
  std::vector<uint16_t> adcs1;
};

using std::string;

using HitTuple = std::tuple<int64_t, uint16_t, uint16_t, int64_t>;
using ClusterTuple = std::tuple<uint16_t, int64_t, uint16_t, int64_t>;
using HitContainer = std::vector<HitTuple>;
using ClusterContainer = std::vector<ClusterTuple>;

using ClusterVectorPlane = std::vector<ClusterPlane>;
using ClusterVectorDetector = std::vector<ClusterDetector>;
using HitVector = std::vector<Hit>;
