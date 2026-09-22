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
**  Configuration.cpp
**
****************************************************************************/


#include <fstream>
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
#include <nlohmann/json.hpp>
#pragma GCC diagnostic pop

#include "Configuration.h"
#include <log.h>
#include <cmath>
#include <cstring>
#include <ctime>
#include <iomanip>
#include <regex>
#include <string>

using json = nlohmann::json;
bool Configuration::PrintUsage(const std::string &errorMessage, char *argv) {
  std::cout << "\nUsage:" << std::endl;
  std::cout
      << "./convertFile -f ../../FAN0_gdgem_readouts_20190528-165706_00000.h5 "
      << "-geo geometry.json "
      << "-th [0,20] -cs [1,2] -ccs [3,4] -dt [200,300] -mst "
         "[1,0] -spc [500,400] "
      << "-dp [200,250] -coin center-of-mass -crl [0.75,0.5] -cru [3.0,5] "
         "-save [[1],[1,2],[1,2]] "
         "-swap 0 -json 0 -n 0 -df 0x44 -cahi 1 -hm 0 -t0 1721234540"
      << std::endl;

  std::cout << "\n\nFlags:\n" << std::endl;
  std::cout << "-f:     PCAPNG file created with wireshark or tcdump "
               "(*.pcapng), coming from the VMM or the CAEN readout.\n"
            << std::endl;
  std::cout
      << "Definition of detector geometry: EITHER the flags -vmm, -axis and "
         "-mapping can be used, OR a JSON geometry file loaded with -geo.\n"
      << std::endl;

  std::cout << "-geo:   Instead of using -vmm, -axis, -map, the detector "
               "geometry can be defined in a JSON file.\n"
            << "        Two examples of geometry files (for strips and pads) "
               "are in the run folder.\n"
            << std::endl;
  std::cout
      << "-bc:    bunch crossing clock. Optional argument (default 40 MHz).\n"
      << std::endl;
  std::cout << "-tac:   tac slope. Optional argument (default 60 ns).\n"
            << std::endl;
  std::cout << "-t0:    Time 0 correction in seconds "
               "The correction value is subtracted from all timestamps.\n"
               "        For VMM data format: if the correction is 0,"
               " the first timestamp of the run is used as correction. "
               "        For SRS data format: SRS runs start at time 0s anyway,"
               " so the correction is only applied if the value is positive. "
               "Optional argument (default 0)\n "
            << std::endl;
  std::cout
      << "-th:    threshold value in ADC counts. Optional argument (default 0, "
         "if -1, only hits with over threshold flag 1 are expected, one value "
         "per detector).\n"
      << std::endl;
  std::cout << "-cs:    minimum cluster size per plane. Optional argument "
               "(default 1), one value per detector.\n"
            << std::endl;
  std::cout << "-ccs:   minimum cluster size in plane 0 and plane 1 together. "
               "Optional argument (default 2), one value per detector.\n"
            << std::endl;
  std::cout
      << "-dt:    maximum time difference between strips in time sorted "
         "vector. Optional argument (default 200), one value per detector.\n"
      << std::endl;
  std::cout << "-mst:   maximum missing strips in strip sorted vector. "
               "Optional argument (default 0), one value per detector.\n"
            << std::endl;
  std::cout
      << "-mp0:   maximum missing pads in dimension 0 in pad sorted "
         "vector. Optional argument (default 0), one value per detector.\n"
      << std::endl;
  std::cout
      << "-mp1:   maximum missing pads in dimension 1 in pad sorted "
         "vector. Optional argument (default 0), one value per detector.\n"
      << std::endl;
  std::cout
      << "-spc:   maximum time span of cluster in one dimension (determined by "
         "drift size and speed). Optional argument (default 500), one value "
         "per detector.\n"
      << std::endl;
  std::cout << "-dp:    maximum time between matched clusters in x and y. "
               "Optional argument (default 200), one value per detector.\n"
            << std::endl;
  std::cout << "-coin:  Valid clusters normally occur at the same time in "
               "plane 0 and plane 1 of a detctor. The parameter -dp determines "
               "the permitted time difference between the planes.\n"
            << "        The time can be calculated with the center-of-mass "
               "algorithm (center-of-mass), the uTPC method (utpc) or the "
               "center-of-mass squared method (charge2).\n"
            << "        Optional argument (default center-of-mass).\n"
            << std::endl;
  std::cout << "-algo:  Select with algorithm is used in pos_algo and "
               "time_algo field in clusters"
            << std::endl;
  std::cout << "        0: utpc with COG" << std::endl;
  std::cout << "        1: utpc with COG2" << std::endl;
  std::cout << "        2: COG including only over Threshold hits" << std::endl;
  std::cout << "        3: COG2 including only over Threshold hits"
            << std::endl;
  std::cout << "        4: position and time of largest ADC" << std::endl;
  std::cout << "        5: trigger pattern (NIP box), the trigger pattern is "
               "stored as integer in time_algo2"
            << std::endl;
  std::cout << "           The vmm that is connected to the NIP box has to be "
               "defined as plane 2 of the detector."
            << std::endl;
  std::cout << "           The channels of the VMM have to be mapped to the "
               "strips in the form:"
            << std::endl;
  std::cout << "           channel representing bit 0 = strip 0, channel for "
               "bit 1  = strip 1 and so on."
            << std::endl;
  std::cout << "        7: time-of-flight (VMM data format)" << std::endl;

  std::cout << "-crl:   Valid clusters normally have the same amount of charge "
               "in both detector planes (ratio of charge plane 0/charge plane "
               "1 is 100% or 1, one value per detector.\n"
            << "        Depending on the readout, the charge sharing can be "
               "different, e.g. in a standard GEM strip readout the total "
               "charge is divided 60/40 between plane 0/ plane 1\n"
            << "        With -crl one sets the lower threshold for the "
               "plane0/plane1 charge ratio. Optional argument (default 0.5)"
            << std::endl;
  std::cout << "-cru:   With -cru one sets the upper threshold for the "
               "plane0/plane1 charge ratio. Optional argument (default 2), one "
               "value per detector.\n"
            << std::endl;
  
  std::cout << "-save:  select which data to store in root file. Input is a "
               "list of lists of detectors, e.g. [[1,2],[1,2],[1,2,3]]."
            << std::endl;
  std::cout << "        first list : detectors for which to write the hits "
               "(hit is a VMM3a channel over threshold)"
            << std::endl;
  std::cout << "        second list : clusters plane" << std::endl;
  std::cout << "        third list : clusters detector" << std::endl;
  std::cout << "        Examples:" << std::endl;
  std::cout << "            [[1,2],[],[]]: hits for detectors 1 and 2 only"
            << std::endl;
  std::cout << "            [[],[],[1,2]]: clusters detector for detector 1 "
               "and 2 only"
            << std::endl;
  std::cout << "            [[2],[1],[1]]: hits for detector 2, clusters "
               "plane, clusters detector for detector 1 \n"
            << std::endl;
  std::cout << "-json:  create a json file of the detector images. Optional "
               "argument (default 1).\n"
            << std::endl;
  std::cout << "-n:     number of hits to analyze. Optional argument (default "
               "0, i.e. all hits).\n"
            << std::endl;
  std::cout << "-cal:   Name of the calibration file. A calibration file is a "
               "JSON file containing an ADC and/or time correction in the form "
               "of a slope and an offset value. Optional parameter.\n"
            << std::endl;
  std::cout
      << "-df:    Data format: The pcap files can have different data formats, "
      << "depending on the firmware and the digtizer.\n"
      << "        CAEN: R5560 digitizer \n"
      << "        VMM: used for assister cards, timestamps are "
         "part of the data"
      << std::endl;
  std::cout
      << "-cahi:    Calibration histograms: If a calibration file is used, "
      << "histograms of the calibrated and uncalibrated adc and time can be "
         "produced.\n"
      << "        With the help of these histograms the effect of the "
         "calibration can "
         "be checked."
      << "        default: 0" << std::endl;
  std::cout << "-info:  Additional info the user wants to be added to the end "
               "of the newly created file name.\n"
            << std::endl;
  std::cout << "XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX"
               "XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX"
            << std::endl;
  if (argv != nullptr) {
    std::cout << "ERROR: " << errorMessage << ": " << argv << std::endl;
  } else {
    std::cout << "ERROR: " << errorMessage << std::endl;
  }
  std::cout << "\nFor meaning of the flags and the correct usage of "
               "convertFile, please see above!"
            << std::endl;
  std::cout << "XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX"
               "XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX"
            << std::endl;

  return false;
}

bool Configuration::ParseCommandLine(int argc, char **argv) {
  corryvreckan::Log::setSection("Configuration");
  if (argc == 1 || argc % 2 == 0) {
    return PrintUsage("Wrong number of arguments!", argv[argc - 1]);
  }
  for (size_t i = 1; i < static_cast<size_t>(argc); i += 2) {
    if (strncmp(argv[i], "-f", 2) == 0) {
      fFound = true;
      pFileName = argv[i + 1];
    } 
    else if (strncmp(argv[i], "-log", 4) == 0) {
      pLogLevel = argv[i + 1];
      auto it = find(pLogLevels.begin(), pLogLevels.end(), pLogLevel);
      if (it == pLogLevels.end()) {
        pLogLevel = "INFO"; 
      }
    } else if (strncmp(argv[i], "-bf", 3) == 0) {
      pUseBunchFile = true;
      pBunchFile = argv[i + 1];
    } else if (strncmp(argv[i], "-bvin", 5) == 0) {
      pUseBunchFile = true;
      pBunchIntensityVariable = argv[i + 1];
    } else if (strncmp(argv[i], "-bvti", 5) == 0) {
      pUseBunchFile = true;
      pBunchTimeVariable = argv[i + 1];
    } else if (strncmp(argv[i], "-bvtr", 5) == 0) {
      pUseBunchFile = true;
      pBunchTree = argv[i + 1];
    } else if (strncmp(argv[i], "-info", 5) == 0) {
      pInfo = argv[i + 1];
    } else if (strncmp(argv[i], "-t0", 3) == 0) {
      pTime0Correction = static_cast<int64_t>(atol(argv[i + 1]));
    } 
    else if (strncmp(argv[i], "-geo", 4) == 0) {
      pGeometryFile = argv[i + 1];
      if (pGeometryFile.find(".json") == std::string::npos) {
        return PrintUsage("The geometry parameter -geo requires a JSON file!",
                          nullptr);
      }
    } else if (strncmp(argv[i], "-tac", 4) == 0) {
      pTAC = atof(argv[i + 1]);
    } else if (strncmp(argv[i], "-th", 3) == 0) {
      GetDetectorParameters(argv[i + 1], pADCThreshold);
    } else if (strncmp(argv[i], "-cs", 3) == 0) {
      GetDetectorParameters(argv[i + 1], pMinClusterSize);
    } else if (strncmp(argv[i], "-ccs", 4) == 0) {
      GetDetectorParameters(argv[i + 1], pCoincidentClusterSize);
    } else if (strncmp(argv[i], "-dt", 3) == 0) {
      GetDetectorParameters(argv[i + 1], pDeltaTimeHits);
    } else if (strncmp(argv[i], "-mst", 4) == 0) {
      GetDetectorParameters(argv[i + 1], pMissingStripsCluster);
    } else if (strncmp(argv[i], "-spc", 4) == 0) {
      GetDetectorParameters(argv[i + 1], pSpanClusterTime);
    } else if (strncmp(argv[i], "-dp", 3) == 0) {
      GetDetectorParameters(argv[i + 1], pDeltaTimePlanes);
    } else if (strncmp(argv[i], "-crl", 4) == 0) {
      GetDetectorParameters(argv[i + 1], pChargeRatioLower);
    } else if (strncmp(argv[i], "-cru", 4) == 0) {
      GetDetectorParameters(argv[i + 1], pChargeRatioUpper);
    } else if (strncmp(argv[i], "-cahi", 5) == 0) {
      if (atoi(argv[i + 1]) == 1) {
        calibrationHistogram = true;
      } else {
        calibrationHistogram = false;
      }
    } else if (strncmp(argv[i], "-save", 5) == 0) {
      std::string parameterString = argv[i + 1];
      char removeChars[] = " ";
      for (size_t n = 0; n < strlen(removeChars); ++n) {
        parameterString.erase(std::remove(parameterString.begin(),
                                          parameterString.end(),
                                          removeChars[n]),
                              parameterString.end());
      }

      parameterString =
          std::regex_replace(parameterString, std::regex("\\[\\["), "; ");
      parameterString =
          std::regex_replace(parameterString, std::regex("\\]\\]"), ";");
      parameterString =
          std::regex_replace(parameterString, std::regex("\\],\\["), "| ");
      parameterString.erase(
          std::remove(parameterString.begin(), parameterString.end(), ';'),
          parameterString.end());
      std::vector<std::string> vTokens;
      std::string token;
      std::istringstream tokenStream(parameterString);
      int n = 0;
      pSaveWhat = 0;
      while (std::getline(tokenStream, token, '|')) {
        vTokens.push_back(token);
        if (token != " ") {
          if (n == 0) {
            pSaveWhat = 1;
          } else if (n == 1) {
            pSaveWhat += 10;
          } else if (n == 2) {
            pSaveWhat += 100;
          }
        }
        n++;
      }
      if (vTokens.size() != 3) {
        return PrintUsage(
            "The -save parameter accepts only a list of lists of detectors "
            "(numbers) in the format [[],[1,2],[1,2,3]]!",
            nullptr);
      }
      pSaveHits.clear();
      pSaveClustersPlane.clear();
      pSaveClustersDetector.clear();
      n = 0;

      for (auto &s : vTokens) {
        std::vector<std::string> v;
        std::string token2;
        for (size_t z = 0; z < strlen(removeChars); ++z) {
          s.erase(std::remove(s.begin(), s.end(), removeChars[z]), s.end());
        }
        std::istringstream tokenStream2(s);
        while (std::getline(tokenStream2, token2, ',')) {
          if (token2 != " ") {
            if (token2.find_first_not_of("0123456789") != std::string::npos) {
              return PrintUsage(
                  "The -save parameter accepts only a list of lists of "
                  "detectors (numbers) in the format [[],[1,2],[1,2,3]]!",
                  nullptr);
            }
          }
          if (n == 0 && token2 != " ") {
            pSaveHits.push_back(static_cast<uint8_t>(std::stoi(token2)));
          } else if (n == 1 && token2 != " ") {
            pSaveClustersPlane.push_back(static_cast<uint8_t>(std::stoi(token2)));
          } else if (n == 2 && token2 != " ") {
            pSaveClustersDetector.push_back(static_cast<uint8_t>(std::stoi(token2)));
          }
        }
        n++;
      }
    } else if (strncmp(argv[i], "-n", 2) == 0) {
      nHits = static_cast<uint64_t>(atoi(argv[i + 1]));
    } else if (strncmp(argv[i], "-cal", 4) == 0) {
      pCalFilename = argv[i + 1];
      useCalibration = true;
    } else if (strncmp(argv[i], "-json", 5) == 0) {
      createJSON = atoi(argv[i + 1]);
    } else if (strncmp(argv[i], "-coin", 5) == 0) {
      pConditionCoincidence = "center-of-mass";
      if (strncmp(argv[i + 1], "utpc", 4) == 0 ||
          strncmp(argv[i + 1], "charge2", 7) == 0) {
        pConditionCoincidence = argv[i + 1];
      }
    } else if (strncmp(argv[i], "-algo", 5) == 0) {
      pAlgo = atoi(argv[i + 1]);
    } 
    else if (strncmp(argv[i], "-buf", 4) == 0) {
      pBufferInterval_ns = atol(argv[i + 1]);
    } else if (strncmp(argv[i], "-df", 3) == 0) {
      std::string s = argv[i + 1];
      sscanf(s.c_str(), "%x", &pDataFormat);
      // VMM
      // TREX 64 (0x40)
      // NMX 68 (0x44)
      // FREIA 72 (0x48)
      // TBL MB 73 (0x49)
      // ESTIA 76 (0x4C)
      // CAEN R5560
      //  Loki 0x30 (48)
      //  TBL He3 0x32 (50)
      //  BIFROST 0x34 (52)
      //  Miracles 0x38 (56)
      //  CSPEC 0x3C (60)
      //  DREAM 0x60 (96)
      std::vector<int> v_valid_values = {0x40, 0x44, 0x48, 0x49, 0x4c,
                                         0x30, 0x32, 0x34, 0x38, 0x3C, 0x10, 0x60};
      auto searchValid =
          std::find(v_valid_values.begin(), v_valid_values.end(), pDataFormat);
      if (searchValid == v_valid_values.end()) {
        return PrintUsage("The data format parameter -df accepts only the "
                          "instruments that use VMM, CAEN R5560 or I-BM!",
                          argv[i + 1]);
      }
    } else {
      return PrintUsage("Wrong type of argument!", argv[i]);
    }
  }
  if (!fFound) {
    return PrintUsage("Data file has to be loaded with -f data.pcapng!",
                      nullptr);
  }

  if (pFileName.find(".pcapng") == std::string::npos) {
    return PrintUsage("Wrong extension: .pcapng file required for data files!",
                      nullptr);
  }
  if (useCalibration && pCalFilename.find(".json") == std::string::npos) {
    return PrintUsage("Wrong extension: .json file required for calibration!",
                      nullptr);
  }
  if (pDataFormat >= 0x40 && pDataFormat <= 0x4C &&
      (pGeometryFile.find(".json") == std::string::npos)) {
    return PrintUsage("Detectors, planes, fecs and VMMs have to be defined, or "
                      "a geometry file loaded!",
                      nullptr);
  }
  pRootFilename = pFileName;
  if (pRootFilename.find(".h5") != std::string::npos) {
    pRootFilename.replace(pRootFilename.size() - 3, pRootFilename.size(), "");
  } else if (pRootFilename.find(".pcapng") != std::string::npos) {
    pRootFilename.replace(pRootFilename.size() - 7, pRootFilename.size(), "");
  }

  auto t = std::time(nullptr);
  auto tm = *std::localtime(&t);
  std::stringstream sTime;
  sTime << std::put_time(&tm, "%Y%m%d%H%M%S");

  std::string strParams = "_";
  strParams += sTime.str();

  if (pInfo.length() > 0) {
    strParams += "_";
    strParams += pInfo;
  }
  strParams += ".root";
  pRootFilename = pRootFilename + strParams;

  return true;
}


bool Configuration::GetDetectorPlane(std::pair<uint8_t, uint8_t> dp) {
  auto searchDetPlane = p_DetPlane_idx.find(dp);
  if (searchDetPlane == p_DetPlane_idx.end()) {
    return false;
  }
  return true;
}

bool Configuration::CreateMapping() {
  if ((pDataFormat >= 0x10) && ((pDataFormat <= 0x3C) || (pDataFormat == 0x60))) {
    pFecs.clear();
    for (int ring = 0; ring < NUM_RINGS; ring++) {
      for (int fec = 0; fec < FENS_PER_RING; fec++) {
        pFecs.push_back(static_cast<uint16_t>(ring * FENS_PER_RING + fec));
      }
    }
    // Dummy fec number for parser errors
    pFecs.push_back(STATISTIC_FEN);
    return true;
  }
  if (pGeometryFile.find(".json") == std::string::npos) {
    return PrintUsage("Geometry definiton missing! Define geometry by using -geo!",
                      nullptr);
  }
  for (int f = 0; f < NUM_FENS; f++) {
    for (int v = 0; v < 16; v++) {
      pDetectors[f][v] = -1;
      pPlanes[f][v] = -1;
      for (int ch = 0; ch < 64; ch++) {
        pPositions[f][v][ch] = -1;
        pPositions[f][v][ch] = -1;
      }
    }
  }
 

  pAxes.clear();
  std::ifstream t(pGeometryFile);

  std::string jsonstring((std::istreambuf_iterator<char>(t)),
                          std::istreambuf_iterator<char>());

  if (!t.good()) {
    return PrintUsage("Invalid JSON file format!", nullptr);
  }
  nlohmann::json Root;
  try {
    Root = nlohmann::json::parse(jsonstring);
  } catch (...) {
    throw std::runtime_error("Invalid Json in geometry file.");
  }
  pVMMs.clear();
  try {
    auto vmm_geos = Root["vmm_geometry"];
    for (auto &geo : vmm_geos) {
      auto fen = geo["fen"].get<uint16_t>();
      auto ring = geo["ring"].get<uint16_t>();
      auto vmm = geo["vmm"].get<uint8_t>();
      auto detector = geo["detector"].get<uint8_t>();
      // std::string labelDetector = geo["label_detector"].get<std::string>();
      // std::string labelPlane = geo["label_plane"].get<std::string>();

      auto strips0 = geo["id"];
      uint8_t plane = 0;
      uint16_t fec = ring * FENS_PER_RING + fen;

      if (strips0.size() != 64) {
        throw std::runtime_error(
            "Wrong lengths of id arrays in geometry file.");
      } else {
        plane = geo["plane"].get<uint8_t>();
        auto searchDetPlane =
            p_DetPlane_idx.find(std::make_pair(detector, plane));
        if (searchDetPlane == p_DetPlane_idx.end()) {
          // Add det/plane pair to the list and set index
          p_DetPlane_idx.emplace(
              std::make_pair(std::make_pair(detector, plane), 0));
        }
        auto searchMap = pAxes.find(std::make_pair(detector, plane));
        if (searchMap == pAxes.end()) {
          pAxes.emplace(std::make_pair(std::make_pair(detector, plane), 0));
        }
        auto searchTuple =
            std::find(std::begin(pVMMs), std::end(pVMMs),
                      std::make_tuple(detector, plane, fec, vmm));
        if (searchTuple == pVMMs.end()) {
          pVMMs.emplace_back(std::make_tuple(detector, plane, fec, vmm));
          auto searchTuple2 = pChannels.find(std::make_pair(detector, plane));
          int strips = 0;
          for (size_t ch = 0; ch < strips0.size(); ch++) {
            int s0 = strips0[ch].get<int>();
            if (s0 > -1) {
              strips++;
            }
          }
          if (searchTuple2 == pChannels.end()) {
            pChannels[std::make_tuple(detector, plane)] = strips;
          } else {
            pChannels[std::make_tuple(detector, plane)] += strips;
          }
        }
      }
      auto searchFec = std::find(std::begin(pFecs), std::end(pFecs), fec);
      if (searchFec == pFecs.end()) {
        pFecs.push_back(fec);
      }

      auto searchDet = pDets.find(detector);
      if (searchDet == pDets.end()) {
        pDets.emplace(detector, pDets.size());
        pChannels0[detector] = 0;
      }

      bool found = false;
      // Search whether there is a new det/plane/fec combination
      for (auto const &searchDetPlaneFec : pDetectorPlane_Fec) {
        if (searchDetPlaneFec.first == std::make_pair(detector, plane) &&
            searchDetPlaneFec.second == fec) {
          found = true;
          break;
        }
      }
      if (found == false) {
        pFec_DetectorPlane.emplace(
          fec, std::make_pair(detector, plane));
        pDetectorPlane_Fec.emplace(
            std::make_pair(std::make_pair(detector, plane), fec));
      }

      // Search whether there is a new fec/chip combination
      auto searchFecChip =
          pFecChip_DetectorPlane.find(std::make_pair(fec, vmm));
      if (searchFecChip == pFecChip_DetectorPlane.end()) {
        pDetectors[fec][vmm] = static_cast<uint8_t>(detector);
        pPlanes[fec][vmm] = static_cast<uint8_t>(plane);
        
        for (size_t ch = 0; ch < strips0.size(); ch++) {
          int s0 = strips0[ch].get<int>();
          pPositions[fec][vmm][ch] = s0;
        }

        // Add the new fec/chip pair to the list
        pFecChip_DetectorPlane.emplace(std::make_pair(
            std::make_pair(fec, vmm), std::make_pair(detector, plane)));
      }
    }
  } catch (const std::exception &exc) {
    throw std::runtime_error("Invalid json while parsing geometry file.");
  }
  
  // Dummy fec number for parser errors
  pFecs.push_back(STATISTIC_FEN);

  bool ret = CheckDetectorParameters("pMinClusterSize", pMinClusterSize);
  if (ret == false) {
    return false;
  }

  ret =
      CheckDetectorParameters("pCoincidentClusterSize", pCoincidentClusterSize);
  if (ret == false) {
    return false;
  }

  ret = CheckDetectorParameters("pDeltaTimeHits", pDeltaTimeHits);
  if (ret == false) {
    return false;
  }

  ret = CheckDetectorParameters("pMissingStripsCluster", pMissingStripsCluster);
  if (ret == false) {
    return false;
  }

  ret = CheckDetectorParameters("pSpanClusterTime", pSpanClusterTime);
  if (ret == false) {
    return false;
  }

  ret = CheckDetectorParameters("pDeltaTimePlanes", pDeltaTimePlanes);
  if (ret == false) {
    return false;
  }

  ret = CheckDetectorParameters("pChargeRatioLower", pChargeRatioLower);
  if (ret == false) {
    return false;
  }

  ret = CheckDetectorParameters("pChargeRatioUpper", pChargeRatioUpper);
  if (ret == false) {
    return false;
  }

  ret = CheckDetectorParameters("pADCThreshold", pADCThreshold);
  if (ret == false) {
    return false;
  }

  return true;
}

void Configuration::GetDetectorParameters(std::string input,
                                          std::vector<double> &v) {
  v.clear();
  std::string parameterString = input;
  char removeChars[] = "[ ]";
  for (unsigned int i = 0; i < strlen(removeChars); ++i) {
    parameterString.erase(std::remove(parameterString.begin(),
                                      parameterString.end(), removeChars[i]),
                          parameterString.end());
  }
  std::string token;
  std::istringstream tokenStream(parameterString);
  while (std::getline(tokenStream, token, ',')) {
    v.push_back(std::stof(token));
  }
}

bool Configuration::CheckDetectorParameters(std::string name,
                                            std::vector<double> &v) {
  if (v.size() == 1) {
    double val = v[0];
    for (size_t n = 1; n < pDets.size(); n++) {
      v.push_back(static_cast<double>(val));
    }
  } else {
    if (v.size() != pDets.size()) {
      return PrintUsage(
          "Wrong number of parameters, one per detector or one for all!",
          (char *)name.c_str());
    }
  }
  return true;
}
