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
  uint16_t ampa;
  uint16_t ampb;
  uint16_t ampc;
  uint16_t ampd;
  uint8_t om;
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
