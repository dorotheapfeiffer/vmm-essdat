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
  double pulse_time;
  double bunch_intensity;
  double time;
  uint8_t geo_id;
  uint8_t ch;
  uint16_t pos;
  uint16_t bcid;
  uint16_t tdc;
  uint16_t adc;
  bool over_threshold;
  double chip_time;
};

struct HitR5560 {
  uint8_t ring;
  uint8_t fen;
  uint8_t group;
  uint32_t counter;
  int16_t ampa;
  int16_t ampb;
  int16_t ampc;
  int16_t ampd;
  uint8_t om;
  double pulse_time;
  double time;
};

struct HitIBM {
  uint8_t ring;
  uint8_t fen;
  uint8_t type;
  float adc_mv;
  float adc;
  uint32_t adc_raw;
  uint16_t samples;
  double pulse_time;
  double time;
};

struct ClusterPlane {
  uint8_t det;
  uint8_t plane;
  double pulse_time;
  double bunch_intensity;
  uint16_t size;
  uint16_t adc;
  double time;
  double time_utpc;
  double time_charge2;
  double time_algo;
  double pos;
  double pos_utpc;
  double pos_charge2;
  double pos_algo;
  bool plane_coincidence;
  uint16_t max_delta_time;
  uint16_t max_missing_strip;
  uint16_t span_cluster;
  std::vector<double> strips;
  std::vector<double> times;
  std::vector<double> adcs;
};

struct ClusterDetector {
  uint8_t det;
  double pulse_time;
  double bunch_intensity;
  uint16_t size0;
  uint16_t size1;
  uint16_t adc0;
  uint16_t adc1;
  double pos0;
  double pos1;
  double time0;
  double time1;
  double pos0_utpc;
  double pos1_utpc;
  double time0_utpc;
  double time1_utpc;
  double pos0_charge2;
  double pos1_charge2;
  double time0_charge2;
  double time1_charge2;
  double pos0_algo;
  double pos1_algo;
  double time0_algo;
  double time1_algo;
  double dt0;
  double dt1;
  double delta_plane_0_1;
  uint16_t span_cluster0;
  uint16_t span_cluster1;
  uint16_t max_delta_time0;
  uint16_t max_delta_time1;
  uint16_t max_missing_strip0;
  uint16_t max_missing_strip1;
  std::vector<double> strips0;
  std::vector<double> times0;
  std::vector<double> adcs0;
  std::vector<double> strips1;
  std::vector<double> times1;
  std::vector<double> adcs1;
};

using std::string;

using HitTuple = std::tuple<double, uint16_t, uint16_t, double>;
using ClusterTuple = std::tuple<uint16_t, double, uint16_t, double>;
using HitContainer = std::vector<HitTuple>;
using ClusterContainer = std::vector<ClusterTuple>;

using ClusterVectorPlane = std::vector<ClusterPlane>;
using ClusterVectorDetector = std::vector<ClusterDetector>;
using HitVector = std::vector<Hit>;
