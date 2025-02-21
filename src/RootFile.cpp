#include "RootFile.h"
#include "TBufferJSON.h"
#include "TMath.h"
#include <RooDouble.h>
#include <TStyle.h>
#include <time.h>

RootFile *RootFile::m_rootFile = nullptr;

RootFile *RootFile::GetInstance() { return m_rootFile; }

RootFile *RootFile::GetInstance(Configuration &config) {
  if (!m_rootFile) {
    m_rootFile = new RootFile(config);
  }

  return m_rootFile;
}

void RootFile::Dispose() {
  if (m_rootFile) {
    m_rootFile->WriteRootFile();
    delete m_rootFile;
    m_rootFile = 0;
  }
}

void RootFile::WriteRootFile() {
  if (m_config.pSaveWhat >= 100) {
    SaveHistograms();
  }
  m_file->Write("", TObject::kOverwrite);
  m_file->Close();
}

void RootFile::SaveDate(double the_seconds_start, std::string the_date_start,
                        double the_seconds_end, std::string the_date_end,
                        uint64_t num_triggers) {
  TString str_date_start = the_date_start;
  TString str_time_start = Form("%f", the_seconds_start);
  TNamed unixtime;
  unixtime.SetName("unixtime");
  unixtime.SetTitle(str_time_start);
  unixtime.Write();

  TNamed datetime;
  datetime.SetName("date");
  datetime.SetTitle(str_date_start);
  datetime.Write();

  TString str_date_end = the_date_end;
  TString str_time_end = Form("%f", the_seconds_end);
  double seconds_duration = the_seconds_end - the_seconds_start;

  TString str_duration = Form("%f", seconds_duration);
  TString str_triggers = Form("%llu", num_triggers);
  TNamed unixtime_end;
  unixtime_end.SetName("unixtime_end");
  unixtime_end.SetTitle(str_time_end);
  unixtime_end.Write();

  TNamed datetime_end;
  datetime_end.SetName("date_end");
  datetime_end.SetTitle(str_date_end);
  datetime_end.Write();

  TNamed duration;
  duration.SetName("duration");
  duration.SetTitle(str_duration);
  duration.Write();

  TNamed triggers;
  triggers.SetName("triggers");
  triggers.SetTitle(str_triggers);
  triggers.Write();

  TNamed channelMapping;
  channelMapping.SetName("channel mapping");
  channelMapping.SetTitle(m_config.pChannelMapping.c_str());
  channelMapping.Write();

  TString str_correction = Form("%f [s]", m_config.pTime0Correction);
  TNamed t0Correction;
  t0Correction.SetName("t0Correction");
  t0Correction.SetTitle(str_correction);
  t0Correction.Write();

  std::stringstream s;

  TNamed chargeRatioLower;
  chargeRatioLower.SetName("charge_ratio_lower");
  std::copy(m_config.pChargeRatioLower.begin(),
            m_config.pChargeRatioLower.end(),
            std::ostream_iterator<double>(s, ", "));
  std::string str = s.str();
  if (str.length() >= 2 && str.substr(str.length() - 2) == ", ") {
    str.erase(str.length() - 2);
  }
  s.str(std::string());
  chargeRatioLower.SetTitle(str.c_str());
  chargeRatioLower.Write();

  TNamed chargeRatioUpper;
  chargeRatioUpper.SetName("charge_ratio_upper");
  std::copy(m_config.pChargeRatioUpper.begin(),
            m_config.pChargeRatioUpper.end(),
            std::ostream_iterator<double>(s, ", "));
  str = s.str();
  if (str.length() >= 2 && str.substr(str.length() - 2) == ", ") {
    str.erase(str.length() - 2);
  }
  s.str(std::string());
  chargeRatioUpper.SetTitle(str.c_str());
  chargeRatioUpper.Write();

  TNamed bc;
  bc.SetName("bc");
  std::string sBC = std::to_string(m_config.pBC);
  bc.SetTitle(sBC.c_str());
  bc.Write();

  TNamed tac;
  tac.SetName("tac");
  std::string sTAC = std::to_string(m_config.pTAC);
  tac.SetTitle(sTAC.c_str());
  tac.Write();

  TNamed minClusterSize;
  minClusterSize.SetName("min_cluster_size");
  std::copy(m_config.pMinClusterSize.begin(), m_config.pMinClusterSize.end(),
            std::ostream_iterator<int>(s, ", "));
  str = s.str();
  if (str.length() >= 2 && str.substr(str.length() - 2) == ", ") {
    str.erase(str.length() - 2);
  }
  s.str(std::string());
  minClusterSize.SetTitle(str.c_str());
  minClusterSize.Write();

  TNamed coincidentClusterSize;
  coincidentClusterSize.SetName("coincident_cluster_size");
  std::copy(m_config.pCoincidentClusterSize.begin(),
            m_config.pCoincidentClusterSize.end(),
            std::ostream_iterator<int>(s, ", "));
  str = s.str();
  if (str.length() >= 2 && str.substr(str.length() - 2) == ", ") {
    str.erase(str.length() - 2);
  }
  s.str(std::string());
  coincidentClusterSize.SetTitle(str.c_str());
  coincidentClusterSize.Write();

  TNamed deltaTimeHits;
  deltaTimeHits.SetName("delta_time_hits");
  std::copy(m_config.pDeltaTimeHits.begin(), m_config.pDeltaTimeHits.end(),
            std::ostream_iterator<double>(s, ", "));
  str = s.str();
  if (str.length() >= 2 && str.substr(str.length() - 2) == ", ") {
    str.erase(str.length() - 2);
  }
  s.str(std::string());
  deltaTimeHits.SetTitle(str.c_str());
  deltaTimeHits.Write();

  TNamed missingStripsCluster;
  missingStripsCluster.SetName("missing_strips");
  std::copy(m_config.pMissingStripsCluster.begin(),
            m_config.pMissingStripsCluster.end(),
            std::ostream_iterator<int>(s, ", "));
  str = s.str();
  if (str.length() >= 2 && str.substr(str.length() - 2) == ", ") {
    str.erase(str.length() - 2);
  }
  s.str(std::string());
  missingStripsCluster.SetTitle(str.c_str());
  missingStripsCluster.Write();
  s.str(std::string());

  TNamed spanClusterTime;
  spanClusterTime.SetName("span_cluster_time");
  std::copy(m_config.pSpanClusterTime.begin(), m_config.pSpanClusterTime.end(),
            std::ostream_iterator<double>(s, ", "));
  str = s.str();
  if (str.length() >= 2 && str.substr(str.length() - 2) == ", ") {
    str.erase(str.length() - 2);
  }
  s.str(std::string());
  spanClusterTime.SetTitle(str.c_str());
  spanClusterTime.Write();

  TNamed deltaTimePlanes;
  deltaTimePlanes.SetName("delta_time_planes");
  std::copy(m_config.pDeltaTimePlanes.begin(), m_config.pDeltaTimePlanes.end(),
            std::ostream_iterator<double>(s, ", "));
  str = s.str();
  if (str.length() >= 2 && str.substr(str.length() - 2) == ", ") {
    str.erase(str.length() - 2);
  }
  s.str(std::string());
  deltaTimePlanes.SetTitle(str.c_str());
  deltaTimePlanes.Write();
}

void RootFile::FillCalibHistos(uint16_t fec, uint8_t vmm, uint8_t ch, float adc,
                               float adc_corrected, float chip_time,
                               float chip_time_corrected) {
  if (m_config.pDataFormat < 0x40 || m_config.pDataFormat > 0x4C) {
    return;
  }
  if (m_map_calib_TH2D.find(std::make_tuple(fec, vmm, "adc_without_calib")) !=
      m_map_calib_TH2D.end()) {
    int idx = m_map_calib_TH2D[std::make_tuple(fec, vmm, "adc_without_calib")];
    m_calib_TH2D[idx]->Fill(ch, adc);
  }
  if (m_map_calib_TH2D.find(std::make_tuple(fec, vmm, "adc_with_calib")) !=
      m_map_calib_TH2D.end()) {
    int idx = m_map_calib_TH2D[std::make_tuple(fec, vmm, "adc_with_calib")];
    m_calib_TH2D[idx]->Fill(ch, adc_corrected);
  }
  if (m_map_calib_TH2D.find(std::make_tuple(fec, vmm, "time_without_calib")) !=
      m_map_calib_TH2D.end()) {
    int idx = m_map_calib_TH2D[std::make_tuple(fec, vmm, "time_without_calib")];
    m_calib_TH2D[idx]->Fill(ch, chip_time);
  }
  if (m_map_calib_TH2D.find(std::make_tuple(fec, vmm, "time_with_calib")) !=
      m_map_calib_TH2D.end()) {
    int idx = m_map_calib_TH2D[std::make_tuple(fec, vmm, "time_with_calib")];
    m_calib_TH2D[idx]->Fill(ch, chip_time_corrected);
  }
}

void RootFile::CreateCAENHistos() {
  TH1D *h1;
  std::string name = "";
  /*
  for (int ring = 0; ring < NUM_RINGS; ring++) {
    name = "ring" + std::to_string(ring);
    h1 = new TH1D(name.c_str(), name.c_str(), 2020, -10, 1000);
    m_delta_t_ring.push_back(h1);
    m_lastTimeRing.push_back(0);
    for (int fen = 0; fen < FENS_PER_RING; fen++) {
      name = "ring" + std::to_string(ring) + "_fen" + std::to_string(fen);
      h1 = new TH1D(name.c_str(), name.c_str(), 2020, -10, 1000);
      m_delta_t_fen.push_back(h1);
      m_lastTimeFen.push_back(0);
      for (int tube = 0; tube < 16; tube++) {
        name = "ring" + std::to_string(ring) + "_fen" + std::to_string(fen) +
               "_tube" + std::to_string(tube);
        h1 = new TH1D(name.c_str(), name.c_str(), 2020, -10, 1000);
        m_delta_t_tube.push_back(h1);
        m_lastTimeTube.push_back(0);
      }
    }
  }
*/
  m_tree_hits = new TTree("hits", "hits");
  m_tree_hits->SetDirectory(m_file);
  m_tree_hits->Branch("hits", &m_hit_r5560);
  return;
}

void RootFile::CreateMonitoringHistos() {}

void RootFile::CreateCalibHistos() {
  TH2D *h2;
  std::string name = "";
  int cntCal = 0;
  for (int i = 0; i < m_config.pVMMs.size(); i++) {
    auto tuple = m_config.pVMMs[i];
    auto det = std::get<0>(tuple);
    auto plane = std::get<1>(tuple);
    auto fec = std::get<2>(tuple);
    auto vmm = std::get<3>(tuple);

    name = "fec" + std::to_string(fec) + "_vmm" + std::to_string(vmm) +
           "_adc_with_calib";
    h2 = new TH2D(name.c_str(), name.c_str(), 64, 0, 64, 1023, 0, 1023);
    m_calib_TH2D.push_back(h2);
    m_map_calib_TH2D.emplace(
        std::make_pair(std::make_tuple(fec, vmm, "adc_with_calib"), cntCal));
    cntCal++;
    name = "fec" + std::to_string(fec) + "_vmm" + std::to_string(vmm) +
           "_adc_without_calib";
    h2 = new TH2D(name.c_str(), name.c_str(), 64, 0, 64, 1023, 0, 1023);
    m_calib_TH2D.push_back(h2);
    m_map_calib_TH2D.emplace(
        std::make_pair(std::make_tuple(fec, vmm, "adc_without_calib"), cntCal));
    cntCal++;
    name = "fec" + std::to_string(fec) + "_vmm" + std::to_string(vmm) +
           "_time_with_calib";
    h2 = new TH2D(name.c_str(), name.c_str(), 64, 0, 64, 1000, -50, 50);
    m_calib_TH2D.push_back(h2);
    m_map_calib_TH2D.emplace(
        std::make_pair(std::make_tuple(fec, vmm, "time_with_calib"), cntCal));
    cntCal++;
    name = "fec" + std::to_string(fec) + "_vmm" + std::to_string(vmm) +
           "_time_without_calib";
    h2 = new TH2D(name.c_str(), name.c_str(), 64, 0, 64, 1000, -50, 50);
    m_calib_TH2D.push_back(h2);
    m_map_calib_TH2D.emplace(std::make_pair(
        std::make_tuple(fec, vmm, "time_without_calib"), cntCal));
    cntCal++;
  }
}

RootFile::RootFile(Configuration &config) : m_config(config) {
  m_fileName = m_config.pRootFilename.c_str();
  m_file = TFile::Open(m_fileName, "RECREATE");
  m_eventNr = 0;
  if (m_config.pDataFormat >= 0x30 && m_config.pDataFormat <= 0x3C) {
    CreateCAENHistos();
    return;
  }
  if (m_config.useCalibration && m_config.calibrationHistogram) {
    CreateCalibHistos();
  }
  if (m_config.monitoringHistogram) {
    CreateMonitoringHistos();
  }
  switch (m_config.pSaveWhat) {
  case 1:
    m_tree_hits = new TTree("hits", "hits");
    m_tree_hits->SetDirectory(m_file);
    m_tree_hits->Branch("hits", &m_hit);
    break;
  case 10:
    m_tree_clusters_plane = new TTree("clusters_plane", "clusters plane");
    m_tree_clusters_plane->SetDirectory(m_file);
    m_tree_clusters_plane->Branch("clusters_plane", &m_cluster_plane);
    break;
  case 11:
    m_tree_hits = new TTree("hits", "hits");
    m_tree_hits->SetDirectory(m_file);
    m_tree_clusters_plane = new TTree("clusters_plane", "clusters plane");
    m_tree_clusters_plane->SetDirectory(m_file);
    m_tree_hits->Branch("hits", &m_hit);
    m_tree_clusters_plane->Branch("clusters_plane", &m_cluster_plane);
    break;
  case 100:
    m_tree_clusters_detector =
        new TTree("clusters_detector", "clusters detector");
    m_tree_clusters_detector->SetDirectory(m_file);
    m_tree_clusters_detector->Branch("clusters_detector", &m_cluster_detector);
    break;
  case 101:
    m_tree_hits = new TTree("hits", "hits");
    m_tree_hits->SetDirectory(m_file);
    m_tree_clusters_detector =
        new TTree("clusters_detector", "clusters detector");
    m_tree_clusters_detector->SetDirectory(m_file);
    m_tree_hits->Branch("hits", &m_hit);
    m_tree_clusters_detector->Branch("clusters_detector", &m_cluster_detector);
    break;
  case 110:
    m_tree_clusters_plane = new TTree("clusters_plane", "clusters plane");
    m_tree_clusters_plane->SetDirectory(m_file);
    m_tree_clusters_detector =
        new TTree("clusters_detector", "clusters detector");
    m_tree_clusters_detector->SetDirectory(m_file);
    m_tree_clusters_plane->Branch("clusters_plane", &m_cluster_plane);
    m_tree_clusters_detector->Branch("clusters_detector", &m_cluster_detector);
    break;
  case 111:
    m_tree_hits = new TTree("hits", "hits");
    m_tree_hits->SetDirectory(m_file);
    m_tree_clusters_plane = new TTree("clusters_plane", "clusters plane");
    m_tree_clusters_plane->SetDirectory(m_file);
    m_tree_clusters_detector =
        new TTree("clusters_detector", "clusters detector");
    m_tree_clusters_detector->SetDirectory(m_file);
    m_tree_hits->Branch("hits", &m_hit);
    m_tree_clusters_plane->Branch("clusters_plane", &m_cluster_plane);
    m_tree_clusters_detector->Branch("clusters_detector", &m_cluster_detector);

    break;
  }

  if (m_config.pSaveWhat >= 100) {
    TH2D *h2;
    TH1D *h1;
    std::string name = "";
    int cnt1D = 0;
    int cnt2D = 0;

    m_max0 = -999999999;
    m_max1 = -999999999;
    m_min0 = 9999999;
    m_min1 = 9999999;
    m_bins0 = 0;
    m_bins1 = 0;

    for (auto const &det : m_config.pDets) {
      auto dp0 = std::make_pair(det.first, 0);
      auto dp1 = std::make_pair(det.first, 1);
      // 2D detectors
      if (m_config.GetDetectorPlane(dp0) == true &&
          m_config.GetDetectorPlane(dp1) == true) {
        int n0 = m_config.pChannels[dp0];
        int n1 = m_config.pChannels[dp1];

        m_min0 = 0;
        m_min1 = 0;
        m_max0 = n0;
        m_max1 = n1;
        m_bins0 = n0 * BINNING_FACTOR;
        m_bins1 = n1 * BINNING_FACTOR;

        name = std::to_string(det.first) + "_delta_time_planes";
        h1 = new TH1D(name.c_str(), name.c_str(), 1000, -500, 500);
        m_TH1D.push_back(h1);
        m_map_TH1D.emplace(std::make_pair(
            std::make_pair(det.first, "delta_time_planes"), cnt1D));
        cnt1D++;

        name = std::to_string(det.first) + "_delta_time_utpc_planes";
        h1 = new TH1D(name.c_str(), name.c_str(), 1000, -500, 500);
        m_TH1D.push_back(h1);
        m_map_TH1D.emplace(std::make_pair(
            std::make_pair(det.first, "delta_time_utpc_planes"), cnt1D));
        cnt1D++;

        name = std::to_string(det.first) + "_delta_time_charge2_planes";
        h1 = new TH1D(name.c_str(), name.c_str(), 1000, -500, 500);
        m_TH1D.push_back(h1);
        m_map_TH1D.emplace(std::make_pair(
            std::make_pair(det.first, "delta_time_charge2_planes"), cnt1D));
        cnt1D++;

        name = std::to_string(det.first) + "_dt0";
        h1 = new TH1D(name.c_str(), name.c_str(), 11000, -1000, 109000);
        m_TH1D.push_back(h1);
        m_map_TH1D.emplace(
            std::make_pair(std::make_pair(det.first, "dt0"), cnt1D));
        cnt1D++;

        name = std::to_string(det.first) + "_dt1";
        h1 = new TH1D(name.c_str(), name.c_str(), 11000, -1000, 109000);
        m_TH1D.push_back(h1);
        m_map_TH1D.emplace(
            std::make_pair(std::make_pair(det.first, "dt1"), cnt1D));
        cnt1D++;

        name = std::to_string(det.first) + "_cluster";
        h2 = new TH2D(name.c_str(), name.c_str(), m_bins0, m_min0, m_max0,
                      m_bins1, m_min1, m_max1);

        h2->GetXaxis()->SetTitle("x");
        h2->GetYaxis()->SetTitle("y");
        m_TH2D.push_back(h2);
        m_map_TH2D.emplace(
            std::make_pair(std::make_pair(det.first, "cluster"), cnt2D));

        cnt2D++;

        name = std::to_string(det.first) + "_cluster_utpc";
        h2 = new TH2D(name.c_str(), name.c_str(), m_bins0, m_min0, m_max0,
                      m_bins1, m_min1, m_max1);
        m_TH2D.push_back(h2);
        m_map_TH2D.emplace(
            std::make_pair(std::make_pair(det.first, "cluster_utpc"), cnt2D));
        cnt2D++;

        name = std::to_string(det.first) + "_cluster_charge2";
        h2 = new TH2D(name.c_str(), name.c_str(), m_bins0, m_min0, m_max0,
                      m_bins1, m_min1, m_max1);
        m_TH2D.push_back(h2);
        m_map_TH2D.emplace(std::make_pair(
            std::make_pair(det.first, "cluster_charge2"), cnt2D));
        cnt2D++;

        name = std::to_string(det.first) + "_cluster_algo";
        h2 = new TH2D(name.c_str(), name.c_str(), m_bins0, m_min0, m_max0,
                      m_bins1, m_min1, m_max1);
        m_TH2D.push_back(h2);
        m_map_TH2D.emplace(
            std::make_pair(std::make_pair(det.first, "cluster_algo"), cnt2D));
        cnt2D++;

        name = std::to_string(det.first) + "_size_plane0";
        h2 = new TH2D(name.c_str(), name.c_str(), m_bins0, m_min0, m_max0,
                      m_bins1, m_min1, m_max1);
        m_TH2D.push_back(h2);
        m_map_TH2D.emplace(
            std::make_pair(std::make_pair(det.first, "size_plane0"), cnt2D));
        cnt2D++;

        name = std::to_string(det.first) + "_size_plane1";
        h2 = new TH2D(name.c_str(), name.c_str(), m_bins0, m_min0, m_max0,
                      m_bins1, m_min1, m_max1);
        m_TH2D.push_back(h2);
        m_map_TH2D.emplace(
            std::make_pair(std::make_pair(det.first, "size_plane1"), cnt2D));
        cnt2D++;

        name = std::to_string(det.first) + "_size_plane01";
        h2 = new TH2D(name.c_str(), name.c_str(), m_bins0, m_min0, m_max0,
                      m_bins1, m_min1, m_max1);
        m_TH2D.push_back(h2);
        m_map_TH2D.emplace(
            std::make_pair(std::make_pair(det.first, "size_plane01"), cnt2D));
        cnt2D++;

        name = std::to_string(det.first) + "_charge_plane0";
        h2 = new TH2D(name.c_str(), name.c_str(), m_bins0, m_min0, m_max0,
                      m_bins1, m_min1, m_max1);
        m_TH2D.push_back(h2);
        m_map_TH2D.emplace(
            std::make_pair(std::make_pair(det.first, "charge_plane0"), cnt2D));
        cnt2D++;

        name = std::to_string(det.first) + "_charge_plane1";
        h2 = new TH2D(name.c_str(), name.c_str(), m_bins0, m_min0, m_max0,
                      m_bins1, m_min1, m_max1);
        m_TH2D.push_back(h2);
        m_map_TH2D.emplace(
            std::make_pair(std::make_pair(det.first, "charge_plane1"), cnt2D));
        cnt2D++;

        name = std::to_string(det.first) + "_charge_plane01";
        h2 = new TH2D(name.c_str(), name.c_str(), m_bins0, m_min0, m_max0,
                      m_bins1, m_min1, m_max1);
        m_TH2D.push_back(h2);
        m_map_TH2D.emplace(
            std::make_pair(std::make_pair(det.first, "charge_plane01"), cnt2D));
        cnt2D++;
      }
    }
  }

  std::cout << "ROOT file " << m_fileName << " created!" << std::endl;
}

RootFile::~RootFile() {}

void RootFile::AddHits(Hit &&the_hit) { m_hits.emplace_back(the_hit); }
void RootFile::AddHits(HitR5560 &&the_hit) {
  m_hits_r5560.emplace_back(the_hit);
}

void RootFile::SaveHits() {
  if (m_hits.size() > 0) {
    for (int n = 0; n < m_hits.size(); n++) {
      m_hit = m_hits[n];
      m_tree_hits->Fill();
    }
    m_hits.clear();
  }
  if (m_hits_r5560.size() > 0) {
    std::sort(begin(m_hits_r5560), end(m_hits_r5560),
              [](const HitR5560 &t1, const HitR5560 &t2) {
                return t1.time < t2.time;
              });
    for (int n = 0; n < m_hits_r5560.size(); n++) {
      m_hit_r5560 = m_hits_r5560[n];
      m_tree_hits->Fill();
    }
    m_hits_r5560.clear();
  }
}

void RootFile::SaveClustersPlane(ClusterVectorPlane &&clusters_plane) {
  if (clusters_plane.size() > 0) {
    for (auto &it : clusters_plane) {
      if (std::find(m_config.pSaveClustersPlane.begin(),
                    m_config.pSaveClustersPlane.end(),
                    it.det) != m_config.pSaveClustersPlane.end()) {
        auto detector_plane = std::make_pair(it.det, it.plane);
        if (m_config.GetDetectorPlane(detector_plane) ||
            m_config.pIsPads[it.det]) {
          m_cluster_plane = it;
          m_tree_clusters_plane->Fill();
        }
      }
    }
  }
}

void RootFile::SaveClustersDetector(ClusterVectorDetector &&clusters_detector) {
  for (auto &it : clusters_detector) {

    if (std::find(m_config.pSaveClustersDetector.begin(),
                  m_config.pSaveClustersDetector.end(),
                  it.det) != m_config.pSaveClustersDetector.end()) {
      auto dp0 = std::make_pair(it.det, 0);
      auto dp1 = std::make_pair(it.det, 1);
      // 2D detector
      if (m_config.GetDetectorPlane(dp0) == true &&
          m_config.GetDetectorPlane(dp1) == true) {
        int idx = m_map_TH1D[std::make_pair(it.det, "delta_time_planes")];
        m_TH1D[idx]->Fill(it.time0 - it.time1);
        idx = m_map_TH1D[std::make_pair(it.det, "delta_time_utpc_planes")];
        m_TH1D[idx]->Fill(it.time0_utpc - it.time1_utpc);

        idx = m_map_TH1D[std::make_pair(it.det, "delta_time_charge2_planes")];
        m_TH1D[idx]->Fill(it.time0_charge2 - it.time1_charge2);

        idx = m_map_TH1D[std::make_pair(it.det, "dt0")];
        m_TH1D[idx]->Fill(it.dt0);

        idx = m_map_TH1D[std::make_pair(it.det, "dt1")];
        m_TH1D[idx]->Fill(it.dt1);

        idx = m_map_TH2D[std::make_pair(it.det, "cluster")];
        m_TH2D[idx]->Fill(it.pos0, it.pos1);

        idx = m_map_TH2D[std::make_pair(it.det, "cluster_utpc")];
        m_TH2D[idx]->Fill(it.pos0_utpc, it.pos1_utpc);

        idx = m_map_TH2D[std::make_pair(it.det, "cluster_charge2")];
        m_TH2D[idx]->Fill(it.pos0_charge2, it.pos1_charge2);

        idx = m_map_TH2D[std::make_pair(it.det, "cluster_algo")];
        m_TH2D[idx]->Fill(it.pos0_algo, it.pos1_algo);

        idx = m_map_TH2D[std::make_pair(it.det, "size_plane0")];
        m_TH2D[idx]->Fill(it.pos0, it.pos1, it.size0);

        idx = m_map_TH2D[std::make_pair(it.det, "size_plane1")];
        m_TH2D[idx]->Fill(it.pos0, it.pos1, it.size1);

        idx = m_map_TH2D[std::make_pair(it.det, "size_plane01")];
        m_TH2D[idx]->Fill(it.pos0, it.pos1, it.size0 + it.size1);

        idx = m_map_TH2D[std::make_pair(it.det, "charge_plane0")];
        m_TH2D[idx]->Fill(it.pos0, it.pos1, it.adc0);

        idx = m_map_TH2D[std::make_pair(it.det, "charge_plane1")];
        m_TH2D[idx]->Fill(it.pos0, it.pos1, it.adc1);

        idx = m_map_TH2D[std::make_pair(it.det, "charge_plane01")];
        m_TH2D[idx]->Fill(it.pos0, it.pos1, it.adc0 + it.adc1);
      }

      m_cluster_detector = it;

      m_tree_clusters_detector->Fill();
    }
  }
}

void RootFile::SaveHistograms() {
  if (m_config.pDataFormat >= 0x30 && m_config.pDataFormat <= 0x3C) {
    return;
  }
  for (auto const &h1 : m_TH1D) {
    h1->Write("", TObject::kOverwrite);
  }
  for (auto const &det : m_config.pDets) {
    auto dp0 = std::make_pair(det.first, 0);
    auto dp1 = std::make_pair(det.first, 1);

    if (m_config.GetDetectorPlane(dp0) && m_config.GetDetectorPlane(dp1)) {
      if (m_config.createJSON) {
        int id = m_map_TH2D[std::make_pair(det.first, "cluster")];

        TString jsonFilename = m_fileName;
        jsonFilename.ReplaceAll(".root", "");

        TString json = TBufferJSON::ToJSON(m_TH2D[id], 3);
        std::ofstream f1;
        f1.open(jsonFilename + "_detector" + std::to_string(det.first) +
                    "_cluster.json",
                std::ios::out);
        f1 << json;
        f1.close();

        id = m_map_TH2D[std::make_pair(det.first, "cluster_utpc")];
        json = TBufferJSON::ToJSON(m_TH2D[id], 3);
        std::ofstream f2;
        f2.open(jsonFilename + "_detector" + std::to_string(det.first) +
                    "_cluster_utpc.json",
                std::ios::out);
        f2 << json;
        f2.close();

        id = m_map_TH2D[std::make_pair(det.first, "cluster_charge2")];
        json = TBufferJSON::ToJSON(m_TH2D[id], 3);
        std::ofstream f3;
        f3.open(jsonFilename + "_detector" + std::to_string(det.first) +
                    "_cluster_charge2.json",
                std::ios::out);
        f3 << json;
        f3.close();

        id = m_map_TH2D[std::make_pair(det.first, "cluster_algo")];
        json = TBufferJSON::ToJSON(m_TH2D[id], 3);
        std::ofstream f4;
        f4.open(jsonFilename + "_detector" + std::to_string(det.first) +
                    "_cluster_algo.json",
                std::ios::out);
        f4 << json;
        f4.close();
      }
      int n = 0;
      for (int b0 = 1; b0 <= m_bins0; b0++) {
        for (int b1 = 1; b1 <= m_bins1; b1++) {
          int idx = m_map_TH2D[std::make_pair(det.first, "cluster")];
          int cnt = m_TH2D[idx]->GetBinContent(b0, b1);
          double val = 0;
          if (cnt > 0) {
            n++;
            idx = m_map_TH2D[std::make_pair(det.first, "size_plane0")];
            val = m_TH2D[idx]->GetBinContent(b0, b1) / cnt;
            m_TH2D[idx]->SetBinContent(b0, b1, val);

            idx = m_map_TH2D[std::make_pair(det.first, "size_plane1")];
            val = m_TH2D[idx]->GetBinContent(b0, b1) / cnt;
            m_TH2D[idx]->SetBinContent(b0, b1, val);

            idx = m_map_TH2D[std::make_pair(det.first, "size_plane01")];
            val = m_TH2D[idx]->GetBinContent(b0, b1) / cnt;
            m_TH2D[idx]->SetBinContent(b0, b1, val);

            idx = m_map_TH2D[std::make_pair(det.first, "charge_plane0")];
            val = m_TH2D[idx]->GetBinContent(b0, b1) / cnt;
            m_TH2D[idx]->SetBinContent(b0, b1, val);

            idx = m_map_TH2D[std::make_pair(det.first, "charge_plane1")];
            val = m_TH2D[idx]->GetBinContent(b0, b1) / cnt;
            m_TH2D[idx]->SetBinContent(b0, b1, val);

            idx = m_map_TH2D[std::make_pair(det.first, "charge_plane01")];
            val = m_TH2D[idx]->GetBinContent(b0, b1) / cnt;
            m_TH2D[idx]->SetBinContent(b0, b1, val);
          }
        }
      }

      int idx = m_map_TH2D[std::make_pair(det.first, "size_plane0")];
      m_TH2D[idx]->SetEntries(n);
      idx = m_map_TH2D[std::make_pair(det.first, "size_plane1")];
      m_TH2D[idx]->SetEntries(n);
      idx = m_map_TH2D[std::make_pair(det.first, "size_plane01")];
      m_TH2D[idx]->SetEntries(n);
      idx = m_map_TH2D[std::make_pair(det.first, "charge_plane0")];
      m_TH2D[idx]->SetEntries(n);
      idx = m_map_TH2D[std::make_pair(det.first, "charge_plane1")];
      m_TH2D[idx]->SetEntries(n);
      idx = m_map_TH2D[std::make_pair(det.first, "charge_plane01")];
      m_TH2D[idx]->SetEntries(n);
    }
  }
}
