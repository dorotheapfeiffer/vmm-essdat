#include <fstream>
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
#include <nlohmann/json.hpp>
#pragma GCC diagnostic pop

#include "Configuration.h"
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
      << "-vmm "
         "\"[[1,0,2,0],[1,0,2,1],[2,0,2,2],[2,0,2,3],[1,1,2,6],[1,1,2,7],[2,1,"
         "2,8],[2,1,2,9]]\" "
      << "-axis \"[[1,0],0],[[1,1],0]\" -sc \"[[0.4,0.4,1]]\" -tl \"[[-51.2, "
         "-51.2, 100]]\" -ro \"[[0,0,45]]\" -tr \"[[S,T,R2]]\" "
      << "-bc 40 -tac 60 -th [0,20] -cs [1,2] -ccs [3,4] -dt [200,300] -mst "
         "[1,0] -spc [500,400] "
      << "-dp [200,250] -coin center-of-mass -crl [0.75,0.5] -cru [3.0,5] "
         "-save [[1],[1,2],[1,2]] "
         "-swap 0 -json 0 -n 0 -df SRS -cahi 1 -hm 0 -t0 1721234540"
      << std::endl;

  std::cout << "\n\nFlags:\n" << std::endl;
  std::cout << "-f:     PCAPNG file created with wireshark or tcdump "
               "(*.pcapng), coming from the VMM or the CAEN readout.\n"
            << std::endl;
  std::cout
      << "Definition of detector geometry: EITHER the flags -vmm, -axis and "
         "-mapping can be used, OR a JSON geometry file loaded with -geo.\n"
      << std::endl;
  std::cout << "-vmm:   geometry of detectors, plane, fecs and chips starting "
               "and ending with \" and separated by brackets and comma [[det, "
               "plane, fec,chip], [det, plane, fec, chip], etc.]."
            << std::endl;
  std::cout << "        The tuples for the VMMs are defined as follows:"
            << std::endl;
  std::cout << "            detector (choose a number between 0 and 255)"
            << std::endl;
  std::cout << "            plane (0 or 1)" << std::endl;
  std::cout << "            fec (fecID set in firmware based on IP address, "
               "10.0.0.1 is fecID 1, 10.0.0.2 is fecID 2 and so on)"
            << std::endl;
  std::cout << "            vmm (depends on connection of hybrid to FEC, FEC "
               "channel 1 equals VMMs 0 and 1, FEC channel 2 VMMs 2 and 3, FEC "
               "channel 8 VMMs 14 and 15)"
            << std::endl;
  std::cout << "        When looking at the detector, the following "
               "conventions are used:"
            << std::endl;
  std::cout << "            - top side of the hybrids is visible (if the "
               "hybrids are mounted in the readout plane)"
            << std::endl;
  std::cout
      << "            - side of the Hirose connector (bottom of the hybird) is "
         "visible (if hybrids are mounted on the side of the detector)"
      << std::endl;
  std::cout
      << "            - plane 0 is at the bottom (HDMI cables go downwards)"
      << std::endl;
  std::cout << "            - plane 1 is at the right side (HDMI cables go to "
               "the right)"
            << std::endl;
  std::cout << "        If one looks at a VMM3a hybrid (connector to detector "
               "readout is on the bottom side), the channel 0 of the VMM 0 is "
               "always where the HDMI cable is connected"
            << std::endl;
  std::cout << "        If the planes are correctly used as described above, "
               "the VMM IDs are always in icreasing order PER HYBRID (e.g. 14, "
               "15 or e.g. 0, 1)\n"
            << std::endl;

  std::cout << "-axis:  direction of axis. Detector, plane and direction flag "
               "(if direction flag = 1, axis direction is flipped)."
            << std::endl;
  std::cout << "        Detector, plane and direction flag starting and ending "
               "with \" and separated by bracket and comma "
               "[[[det,plane],flag], [[det, plane],flag]]."
            << std::endl;
  std::cout << "        The tuples for the axes are defined as follows:"
            << std::endl;
  std::cout << "            - detector (choose a number between 0 and 255)"
            << std::endl;
  std::cout << "            - plane (0 or 1)" << std::endl;
  std::cout << "            - flip axis flag (0 or 1)" << std::endl;
  std::cout << "        Using the convention described above, if the plane "
               "axis is NOT FLIPPED:"
            << std::endl;
  std::cout << "            - plane 0 is at the bottom and goes from left (0) "
               "to right (255)"
            << std::endl;
  std::cout << "            - plane 1 is at the right and goes from bottom (0) "
               "to top (255)"
            << std::endl;
  std::cout << "        If the plane axis is FLIPPED:" << std::endl;
  std::cout << "            - plane 0 is at the bottom and goes from right "
               "(255) to left (0)"
            << std::endl;
  std::cout << "            - plane 1 is at the right and goes from top (0) to "
               "bottom (255)\n"
            << std::endl;
  std::cout << "-map:   Mapping of VMM3a channels to strips of the detector "
               "readout. There are some pre-defined options:\n"
            << "            - gem: channels are continuously mapped to strips, "
               "channel 0 is strip 0, channel 127 strip 127 (default).\n"
            << "            - gem_swapped: odd and even channels are swapped "
               "(to correct error in readout).\n"
            << "            - mm1: Micromegas mapping from Jona Bortfeld\n"
            << std::endl;
  std::cout << "-geo:   Instead of using -vmm, -axis, -map, the detector "
               "geometry can be defined in a JSON file.\n"
            << "        Two examples of geometry files (for strips and pads) "
               "are in the run folder.\n"
            << std::endl;

  std::cout << "-sc:    Scale coordinates. Per detector a tuple with three "
               "values in mm, e.g for two detectors [[s0,s1,s2], [s0,s1,s2]].\n"
            << std::endl;

  std::cout << "-tl:    Translate coordinates. Per detector a tuple with three "
               "values in mm, e.g for two detectors [[t0,t1,t2], [t0,t1,t2]].\n"
            << std::endl;
  std::cout << "-ro:    Rotate around axis 0, axis 1, axis 2. Per detector "
               "a tuple with three angles in degrees, e.g for two detectors "
               "[[r0,r1,r2], [r0,r1,r2]].\n"
            << std::endl;
  std::cout << "-tr:    Transform detector coordinates. S=scale, T=translate, "
               "R0=rotation axis 0, R1=rotation axis 1, R2=rotation axis 2.\n"
            << "        example (two detectors): -tr [[S,T,R2], [S,T, R2]]. "
               "First scaling, then translation, then rotation around normal "
               "axis to plane0 and plane 1 (axis 2 is normal for a detector "
               "with plane 0 and 1,\nthe rotation is always around the origin "
               "of the coordinate system).\n"
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
               "1 is 100\% or 1, one value per detector.\n"
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
  std::cout
      << "-hm:    High-multiplicity matching mode (values 0 or 1).\n"
         "        During the normal matching vmm-sdat searches for each "
         "cluster in one "
         "plane the best match in the other plane, based on the minimum time "
         "difference between the clusters.\n"
         "        Before considering a cluster in the other plane as match "
         "candidate, "
         "conditions like cluster size, and charge sharing are checked.\n"
         "        In high-multiplicity mode, each combination of clusters in "
         "the two "
         "planes that fulfils the conditions is stored as detector_cluster.\n"
         "        That means each plane cluster can appear several times as "
         "part of a "
         "detector_cluster.\n"
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
  std::cout << "-stats: Show statistics of the run (default 0, do not show any "
               "stats).\n"
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

  if (argc == 1 || argc % 2 == 0) {
    return PrintUsage("Wrong number of arguments!", argv[argc - 1]);
  }
  for (int i = 1; i < argc; i += 2) {
    if (strncmp(argv[i], "-f", 2) == 0) {
      fFound = true;
      pFileName = argv[i + 1];
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
      pTime0Correction = atof(argv[i + 1]);
    } else if (strncmp(argv[i], "-bc", 3) == 0) {
      pBC = atof(argv[i + 1]);
      // ESS VMM firmware has 44.444444 MHz clock
      if (pBC >= 44.4 && pBC <= 44.5) {
        pBCTime_ns = 22.5;
        pOffsetPeriod = 4096.0 * pBCTime_ns;
      }
      // 88.0525 MHz ESS clock, half of that is BC clock
      else if (pBC >= 44.0 && pBC <= 44.1) {
        pBCTime_ns = 22.713721927259;
        pOffsetPeriod = 4096.0 * pBCTime_ns;
      } else {
        pBCTime_ns = (1000.0 / pBC);
        pOffsetPeriod = 4096.0 * 25.0;
      }
    } else if (strncmp(argv[i], "-vmm", 4) == 0) {
      vmmsFound = true;
      if (pGeometryFile.find(".json") != std::string::npos) {
        return PrintUsage("The geometry definiton via the -vmm parameter "
                          "cannot be used together with the -geo parameter!",
                          nullptr);
      }
      std::string vmmString = argv[i + 1];
      char removeChars[] = "[]";
      for (unsigned int i = 0; i < strlen(removeChars); ++i) {
        vmmString.erase(
            std::remove(vmmString.begin(), vmmString.end(), removeChars[i]),
            vmmString.end());
      }
      std::string delims = ",";
      size_t lastOffset = 0;
      pVMMs.clear();

      int n = 0;
      int det = 0;
      int plane = 0;
      int fec = 0;
      int vmm = 0;
      while (true) {
        size_t offset = vmmString.find_first_of(delims, lastOffset);
        if (n % 4 == 0) {
          det = atoi(vmmString.substr(lastOffset, offset - lastOffset).c_str());
        } else if (n % 4 == 1) {
          plane =
              atoi(vmmString.substr(lastOffset, offset - lastOffset).c_str());
        } else if (n % 4 == 2) {
          fec = atoi(vmmString.substr(lastOffset, offset - lastOffset).c_str());
        } else {
          vmm = atoi(vmmString.substr(lastOffset, offset - lastOffset).c_str());
          auto searchTuple = std::find(std::begin(pVMMs), std::end(pVMMs),
                                       std::make_tuple(det, plane, fec, vmm));
          if (searchTuple == pVMMs.end()) {
            pVMMs.emplace_back(std::make_tuple(det, plane, fec, vmm));

            auto searchTuple = pChannels.find(std::make_pair(det, plane));
            if (searchTuple == pChannels.end()) {
              pChannels[std::make_tuple(det, plane)] = 64;
            } else {
              pChannels[std::make_tuple(det, plane)] += 64;
            }
          }
        }
        n++;
        if (offset == std::string::npos) {
          break;
        } else {
          lastOffset = offset + 1; // add one to skip the delimiter
        }
      }
      if (pVMMs.size() != (int)(n / 4)) {
        return PrintUsage("Wrong number of detectors, planes, FECs and VMMs!",
                          argv[i]);
      }
    } else if (strncmp(argv[i], "-axis", 5) == 0) {
      std::string axisString = argv[i + 1];
      char removeChars[] = "[]";
      for (unsigned int i = 0; i < strlen(removeChars); ++i) {
        axisString.erase(
            std::remove(axisString.begin(), axisString.end(), removeChars[i]),
            axisString.end());
      }
      std::string delims = ",";
      size_t lastOffset = 0;
      pAxes.clear();

      int n = 0;
      uint8_t det = 0;
      uint8_t plane = 0;
      uint8_t flip = 0;
      while (true) {
        size_t offset = axisString.find_first_of(delims, lastOffset);
        if (n % 3 == 0) {
          det =
              atoi(axisString.substr(lastOffset, offset - lastOffset).c_str());
        } else if (n % 3 == 1) {
          plane =
              atoi(axisString.substr(lastOffset, offset - lastOffset).c_str());
        } else {
          flip =
              atoi(axisString.substr(lastOffset, offset - lastOffset).c_str());

          auto searchMap = pAxes.find(std::make_pair(det, plane));
          if (searchMap == pAxes.end()) {
            pAxes.emplace(std::make_pair(std::make_pair(det, plane), flip));
          }
        }
        n++;
        if (offset == std::string::npos) {
          break;
        } else {
          lastOffset = offset + 1; // add one to skip the delimiter
        }
      }
      if (pAxes.size() != (int)(n / 3)) {
        return PrintUsage(
            "Wrong number of detectors, planes, direction flag for axis!",
            argv[i]);
      }
    } else if (strncmp(argv[i], "-map", 4) == 0) {
      pChannelMapping = argv[i + 1];
      std::vector<std::string> v_valid_values = {"gem", "gem_swapped", "mm1"};
      auto searchValid = std::find(v_valid_values.begin(), v_valid_values.end(),
                                   pChannelMapping);
      if (searchValid == v_valid_values.end()) {
        return PrintUsage("The mapping parameter -map accepts only the values "
                          "gem, gem_swapped, mm1!",
                          nullptr);
      }
    } else if (strncmp(argv[i], "-geo", 4) == 0) {
      pGeometryFile = argv[i + 1];
      if (pGeometryFile.find(".json") == std::string::npos) {
        return PrintUsage("The geometry parameter -geo requires a JSON file!",
                          nullptr);
      } else {
        if (vmmsFound) {
          return PrintUsage("The geometry parameter -geo cannot be used "
                            "together with the -vmm parameter!",
                            nullptr);
        }
      }
    } else if (strncmp(argv[i], "-tr", 3) == 0) {
      std::string parameterString = argv[i + 1];
      char removeChars[] = " ";
      for (unsigned int i = 0; i < strlen(removeChars); ++i) {
        parameterString.erase(std::remove(parameterString.begin(),
                                          parameterString.end(),
                                          removeChars[i]),
                              parameterString.end());
      }

      parameterString =
          std::regex_replace(parameterString, std::regex("\\[\\["), ";");
      parameterString =
          std::regex_replace(parameterString, std::regex("\\]\\]"), ";");
      parameterString =
          std::regex_replace(parameterString, std::regex("\\],\\["), "|");
      parameterString.erase(
          std::remove(parameterString.begin(), parameterString.end(), ';'),
          parameterString.end());
      std::vector<std::string> vTokens;
      std::string token;
      std::istringstream tokenStream(parameterString);
      while (std::getline(tokenStream, token, '|')) {
        vTokens.push_back(token);
      }

      pTransform.clear();
      for (auto &s : vTokens) {
        std::vector<std::string> v;
        std::string token;
        std::istringstream tokenStream(s);
        while (std::getline(tokenStream, token, ',')) {
          v.push_back(token);
        }
        pTransform.push_back(v);
      }
      int d = 0;
      for (auto &v : pTransform) {
        for (auto &e : v) {
          if (e != "S" && e != "T" && e != "R0" && e != "R1" && e != "R2") {
            return PrintUsage("Wrong parameter in transformation!",
                              argv[i + 1]);
          }
        }
        d++;
      }
    } else if (strncmp(argv[i], "-sc", 3) == 0 ||
               strncmp(argv[i], "-tl", 3) == 0 ||
               strncmp(argv[i], "-ro", 3) == 0) {
      std::string paramString = argv[i + 1];
      char removeChars[] = "[]";
      for (unsigned int i = 0; i < strlen(removeChars); ++i) {
        paramString.erase(
            std::remove(paramString.begin(), paramString.end(), removeChars[i]),
            paramString.end());
      }
      std::string delims = ",";
      size_t lastOffset = 0;

      if (strncmp(argv[i], "-sc", 3) == 0) {
        pScale.clear();
      } else if (strncmp(argv[i], "-tl", 3) == 0) {
        pTranslation.clear();
      } else if (strncmp(argv[i], "-ro", 3) == 0) {
        pRotation.clear();
      }

      int n = 0;
      double p0 = 0;
      double p1 = 0;
      double p2 = 0;
      while (true) {
        size_t offset = paramString.find_first_of(delims, lastOffset);
        if (n % 3 == 0) {
          p0 =
              atof(paramString.substr(lastOffset, offset - lastOffset).c_str());
        } else if (n % 3 == 1) {
          p1 =
              atof(paramString.substr(lastOffset, offset - lastOffset).c_str());
        } else {
          p2 =
              atof(paramString.substr(lastOffset, offset - lastOffset).c_str());

          if (strncmp(argv[i], "-sc", 3) == 0) {
            pScale.emplace_back(std::make_tuple(p0, p1, p2));
          } else if (strncmp(argv[i], "-tl", 3) == 0) {
            pTranslation.emplace_back(std::make_tuple(p0, p1, p2));
          } else if (strncmp(argv[i], "-ro", 3) == 0) {
            pRotation.emplace_back(std::make_tuple(p0, p1, p2));
          }
        }
        n++;
        if (offset == std::string::npos) {
          break;
        } else {
          lastOffset = offset + 1; // add one to skip the delimiter
        }
      }
      if (strncmp(argv[i], "-sc", 3) == 0) {
        if (pScale.size() != (int)(n / 3)) {
          return PrintUsage("Wrong number of scale parameters!", argv[i + 1]);
        }
      } else if (strncmp(argv[i], "-tl", 3) == 0) {
        if (pTranslation.size() != (int)(n / 3)) {
          return PrintUsage("Wrong number of translation parameters!",
                            argv[i + 1]);
        }
      } else if (strncmp(argv[i], "-ro", 3) == 0) {
        if (pRotation.size() != (int)(n / 3)) {
          return PrintUsage("Wrong number of rotation parameters!",
                            argv[i + 1]);
        }
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
      for (unsigned int i = 0; i < strlen(removeChars); ++i) {
        parameterString.erase(std::remove(parameterString.begin(),
                                          parameterString.end(),
                                          removeChars[i]),
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
      pHighMultiplicity = false;

      for (auto &s : vTokens) {
        std::vector<std::string> v;
        std::string token;
        for (unsigned int i = 0; i < strlen(removeChars); ++i) {
          s.erase(std::remove(s.begin(), s.end(), removeChars[i]), s.end());
        }
        std::istringstream tokenStream(s);
        while (std::getline(tokenStream, token, ',')) {
          if (token != " ") {
            if (token.find_first_not_of("0123456789") != std::string::npos) {
              return PrintUsage(
                  "The -save parameter accepts only a list of lists of "
                  "detectors (numbers) in the format [[],[1,2],[1,2,3]]!",
                  nullptr);
            }
          }
          if (n == 0 && token != " ") {
            pSaveHits.push_back(std::stoi(token));
          } else if (n == 1 && token != " ") {
            pSaveClustersPlane.push_back(std::stoi(token));
          } else if (n == 2 && token != " ") {
            pSaveClustersDetector.push_back(std::stoi(token));
          }
        }
        n++;
      }
    } else if (strncmp(argv[i], "-n", 2) == 0) {
      nHits = atoi(argv[i + 1]);
    } else if (strncmp(argv[i], "-stats", 6) == 0) {
      pShowStats = atoi(argv[i + 1]);
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
    } else if (strncmp(argv[i], "-hm", 3) == 0) {
      if (atoi(argv[i + 1]) == 1) {
        pHighMultiplicity = true;
      }
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
      std::vector<int> v_valid_values = {0x40, 0x44, 0x48, 0x49, 0x4c,
                                         0x30, 0x32, 0x34, 0x38, 0x3C};
      auto searchValid =
          std::find(v_valid_values.begin(), v_valid_values.end(), pDataFormat);
      if (searchValid == v_valid_values.end()) {
        std::cout << pDataFormat << std::endl;
        return PrintUsage("The data format parameter -df accepts only the "
                          "instruments that use VMM or CAEN R5560!",
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
      (!vmmsFound && pGeometryFile.find(".json") == std::string::npos)) {
    return PrintUsage("Detectors, planes, fecs and VMMs have to be defined, or "
                      "a geometry file loaded!",
                      nullptr);
  }
  if (pShowStats) {
    std::cout << "Analyzing " << pFileName << " ..." << std::endl;
  }
  pRootFilename = pFileName;
  if (pRootFilename.find(".h5") != std::string::npos) {
    pRootFilename.replace(pRootFilename.size() - 3, pRootFilename.size(), "");
  } else if (pRootFilename.find(".pcapng") != std::string::npos) {
    pRootFilename.replace(pRootFilename.size() - 7, pRootFilename.size(), "");
  }

  time_t ttime = time(0);
  tm *local_time = localtime(&ttime);
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

bool Configuration::CalculateTransform() {
  if (pTransform.size() == 0) {
    return true;
  }
  for (auto &v : pTransform) {
    for (auto &e : v) {
      if (e == "S" && pScale.size() != pDets.size()) {
        return PrintUsage("Scale parameters missing!", nullptr);
      }
      if (e == "T" && pTranslation.size() != pDets.size()) {
        return PrintUsage("Translation parameters missing!", nullptr);
      }
      if ((e == "R0" || e == "R1" || e == "R2") &&
          pRotation.size() != pDets.size()) {
        return PrintUsage("Rotation parameters missing!", nullptr);
      }
    }
  }
  double M[5][16];
  double MR[16];
  double ML[16];
  double MM[16];
  const double pi = std::acos(-1);
  int d = 0;
  int n = 0;

  for (auto &v : pTransform) {
    n = 0;
    for (int i = 0; i < 16; i++) {
      M[0][i] = 0;
      M[1][i] = 0;
      M[2][i] = 0;
      M[3][i] = 0;
      M[4][i] = 0;
    }
    for (auto &e : v) {
      if (e == "S") {
        auto t = pScale[d];
        M[n][0] = std::get<0>(t);
        M[n][5] = std::get<1>(t);
        M[n][10] = std::get<2>(t);
        M[n][15] = 1;
      } else if (e == "T") {
        auto t = pTranslation[d];
        M[n][0] = 1;
        M[n][5] = 1;
        M[n][10] = 1;
        M[n][15] = 1;
        M[n][3] = std::get<0>(t);
        M[n][7] = std::get<1>(t);
        M[n][11] = std::get<2>(t);
      } else if (e == "R0") {
        auto t = pRotation[d];
        double angle = std::get<0>(t) * pi / 180;
        double co = std::cos(angle);
        double si = std::sin(angle);

        M[n][0] = 1;
        M[n][15] = 1;
        M[n][5] = co;
        M[n][6] = -si;
        M[n][9] = si;
        M[n][10] = co;
      } else if (e == "R1" && pRotation.size() != pDets.size()) {
        auto t = pRotation[d];
        double angle = std::get<1>(t) * pi / 180;
        double co = std::cos(angle);
        double si = std::sin(angle);

        M[n][5] = 1;
        M[n][15] = 1;
        M[n][0] = co;
        M[n][2] = si;
        M[n][8] = -si;
        M[n][10] = co;
      } else {
        auto t = pRotation[d];
        double angle = std::get<2>(t) * pi / 180;
        double co = std::cos(angle);
        double si = std::sin(angle);

        M[n][10] = 1;
        M[n][15] = 1;
        M[n][0] = co;
        M[n][1] = -si;
        M[n][4] = si;
        M[n][5] = co;
      }

      n++;
    }
    for (int m = 0; m < n; m++) {
      if (m == 0) {
        memcpy(MR, M[0], sizeof(MR));

      } else if (m > 0) {
        memcpy(ML, M[m], sizeof(ML));

        MM[0] = ML[0] * MR[0] + ML[1] * MR[4] + ML[2] * MR[8] + ML[3] * MR[12];
        MM[1] = ML[0] * MR[1] + ML[1] * MR[5] + ML[2] * MR[9] + ML[3] * MR[13];
        MM[2] = ML[0] * MR[2] + ML[1] * MR[6] + ML[2] * MR[10] + ML[3] * MR[14];
        MM[3] = ML[0] * MR[3] + ML[1] * MR[7] + ML[2] * MR[11] + ML[3] * MR[15];

        MM[4] = ML[4] * MR[0] + ML[5] * MR[4] + ML[6] * MR[8] + ML[7] * MR[12];
        MM[5] = ML[4] * MR[1] + ML[5] * MR[5] + ML[6] * MR[9] + ML[7] * MR[13];
        MM[6] = ML[4] * MR[2] + ML[5] * MR[6] + ML[6] * MR[10] + ML[7] * MR[14];
        MM[7] = ML[4] * MR[3] + ML[5] * MR[7] + ML[6] * MR[11] + ML[7] * MR[15];

        MM[8] =
            ML[8] * MR[0] + ML[9] * MR[4] + ML[10] * MR[8] + ML[11] * MR[12];
        MM[9] =
            ML[8] * MR[1] + ML[9] * MR[5] + ML[10] * MR[9] + ML[11] * MR[13];
        MM[10] =
            ML[8] * MR[2] + ML[9] * MR[6] + ML[10] * MR[10] + ML[11] * MR[14];
        MM[11] =
            ML[8] * MR[3] + ML[9] * MR[7] + ML[10] * MR[11] + ML[11] * MR[15];

        MM[12] =
            ML[12] * MR[0] + ML[13] * MR[4] + ML[14] * MR[8] + ML[15] * MR[12];
        MM[13] =
            ML[12] * MR[1] + ML[13] * MR[5] + ML[14] * MR[9] + ML[15] * MR[13];
        MM[14] =
            ML[12] * MR[2] + ML[13] * MR[6] + ML[14] * MR[10] + ML[15] * MR[14];
        MM[15] =
            ML[12] * MR[3] + ML[13] * MR[7] + ML[14] * MR[11] + ML[15] * MR[15];

        memcpy(MR, MM, sizeof(MR));
      }
    }
    pTransformX.emplace_back(std::make_tuple(MM[0], MM[1], MM[2], MM[3]));
    pTransformY.emplace_back(std::make_tuple(MM[4], MM[5], MM[6], MM[7]));
    pTransformZ.emplace_back(std::make_tuple(MM[8], MM[9], MM[10], MM[11]));

    d++;
  }
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
  if (pDataFormat >= 0x30 && pDataFormat <= 0x3C) {
    pFecs.clear();
    for (int ring = 0; ring < NUM_RINGS; ring++) {
      for (int fec = 0; fec < FENS_PER_RING; fec++) {
        pFecs.push_back(ring * FENS_PER_RING + fec);
      }
    }
    // Dummy fec number for parser errors
    pFecs.push_back(STATISTIC_FEN);
    return true;
  }
  if (pGeometryFile.find(".json") == std::string::npos && !vmmsFound) {
    return PrintUsage("Geometry definiton missing! Define geometry either via "
                      "the -vmm and -axis parameter, or by -geo!",
                      nullptr);
  }
  for (int f = 0; f < NUM_FENS; f++) {
    for (int v = 0; v < 16; v++) {
      pDetectors[f][v] = -1;
      pPlanes[f][v] = -1;
      if (f == 0) {
        pIsPads[v] = false;
      }
      for (int ch = 0; ch < 64; ch++) {
        pPositions[f][v][ch] = -1;
        pPositions[f][v][ch] = -1;
      }
    }
  }
  if (vmmsFound) {
    int lastChip = 0;
    int chip = 0;
    for (int i = 0; i < pVMMs.size(); i++) {
      auto tuple = pVMMs[i];
      auto det = std::get<0>(tuple);
      auto plane = std::get<1>(tuple);
      auto fec = std::get<2>(tuple);
      auto searchFec = std::find(std::begin(pFecs), std::end(pFecs), fec);
      if (searchFec == pFecs.end()) {
        pFecs.push_back(fec);
      }
      lastChip = chip;
      auto chip = std::get<3>(tuple);
      if (i % 2 == 1) {
        int flip = 0;
        if (lastChip > chip) {
          std::string sDet = std::to_string(det);
          std::string sPlane = std::to_string(plane);
          std::string sVMM0 = std::to_string(lastChip);
          std::string sVMM1 = std::to_string(chip);
          std::string sMessage = "Detector " + sDet + ", plane " + sPlane +
                                 ", VMM id(" + sVMM0 + "," + sVMM1 + ")!";
          return PrintUsage("Wrong VMM order for plane!\n" + sMessage, nullptr);
        }
      }

      auto searchDet = pDets.find(det);
      if (searchDet == pDets.end()) {
        pDets.emplace(det, pDets.size());
      }

      bool found = false;
      // Search whether there is a new det/plane/fec combination
      for (auto const &searchDetPlaneFec : pDetectorPlane_Fec) {
        if (searchDetPlaneFec.first == std::make_pair(det, plane) &&
            searchDetPlaneFec.second == fec) {
          found = true;
          break;
        }
      }
      if (found == false) {
        pFec_DetectorPlane.emplace(
          fec, std::make_pair(det, plane));
        pDetectorPlane_Fec.emplace(
            std::make_pair(std::make_pair(det, plane), fec));
      }

      // Search whether there is a new fec/chip combination
      auto searchFecChip =
          pFecChip_DetectorPlane.find(std::make_pair(fec, chip));
      if (searchFecChip == pFecChip_DetectorPlane.end()) {
        // Add the new fec/chip pair to the list
        pFecChip_DetectorPlane.emplace(std::make_pair(
            std::make_pair(fec, chip), std::make_pair(det, plane)));
        // Search for det/plane pairs
        auto searchDetPlane = p_DetPlane_idx.find(std::make_pair(det, plane));
        uint32_t offset = 0;
        if (searchDetPlane == p_DetPlane_idx.end()) {
          // Add det/plane pair to the list and set index
          p_DetPlane_idx.emplace(
              std::make_pair(std::make_pair(det, plane), offset));
        } else {
          // Increment det/plane index
          offset = searchDetPlane->second + 1;
          p_DetPlane_idx[std::make_pair(det, plane)] = offset;
        }
        // Set offset for the new fec/chip combination
        if (pAxes[std::make_pair(det, plane)] == 0) {
          pOffsets.emplace(
              std::make_pair(std::make_pair(fec, chip), offset * 64));
        } else {
          auto channels = pChannels[std::make_tuple(det, plane)];
          pOffsets.emplace(std::make_pair(std::make_pair(fec, chip),
                                          channels - offset * 64));
        }
      }
    }

    for (int f = 0; f < NUM_FENS; f++) {
      for (int v = 0; v < 16; v++) {
        auto searchFecChip = pFecChip_DetectorPlane.find(std::make_pair(f, v));
        if (searchFecChip != pFecChip_DetectorPlane.end()) {
          auto fecChip = std::make_pair(f, v);
          auto det_plane = pFecChip_DetectorPlane[fecChip];
          auto det = std::get<0>(det_plane);
          auto plane = std::get<1>(det_plane);
          pDetectors[f][v] = det;
          pPlanes[f][v] = plane;
          auto flag = pAxes[det_plane];
          auto search = pOffsets.find(fecChip);
          int offset = 0;
          if (search != end(pOffsets)) {
            offset = search->second;
          }
          if (pChannelMapping == "gem_swapped") {
            int channel = 0;
            for (int ch = 0; ch < 64; ch++) {
              if (ch % 2 == 0) {
                channel = ch + 1;
              } else {
                channel = ch - 1;
              }
              if (flag == 1) {
                pPositions[f][v][ch] = offset - channel;
              } else {
                pPositions[f][v][ch] = offset + channel;
              }
            }

          } else if (pChannelMapping == "mm1") {
            int channel = 0;

            for (int ch = 0; ch < 64; ch++) {
              if (ch % 2 == 0) {
                if ((offset / 64) % 2 == 0) {
                  channel = 96 - (ch / 2);
                } else {
                  channel = 64 - (ch / 2);
                }
              } else {
                if ((offset / 64) % 2 == 0) {
                  channel = 97 + (ch - 1) / 2;
                } else {
                  channel = 1 + (ch - 1) / 2;
                }
              }
              pPositions[f][v][ch] = channel;
            }

          } else {
            for (int ch = 0; ch < 64; ch++) {
              if (flag == 1) {
                pPositions[f][v][ch] = offset - ch;
              } else {
                pPositions[f][v][ch] = offset + ch;
              }
            }
          }
        }
      }
    }
  } else {
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
          pIsPads[detector] = false;
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
            auto searchTuple = pChannels.find(std::make_pair(detector, plane));
            int strips = 0;
            for (size_t ch = 0; ch < strips0.size(); ch++) {
              int s0 = strips0[ch].get<int>();
              if (s0 > -1) {
                strips++;
              }
            }
            if (searchTuple == pChannels.end()) {
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
          /*
          pDetectorPlane_Labels.emplace(
              std::make_pair(std::make_pair(detector, plane),
                             std::make_pair(labelDetector, labelPlane)));
          */
        }

        // Search whether there is a new fec/chip combination
        auto searchFecChip =
            pFecChip_DetectorPlane.find(std::make_pair(fec, vmm));
        if (searchFecChip == pFecChip_DetectorPlane.end()) {
          pDetectors[fec][vmm] = (int)detector;
          pPlanes[fec][vmm] = (int)plane;
          
          for (size_t ch = 0; ch < strips0.size(); ch++) {
            int s0 = strips0[ch].get<int>();
            pPositions[fec][vmm][ch] = s0;
          }

          // Add the new fec/chip pair to the list
          pFecChip_DetectorPlane.emplace(std::make_pair(
              std::make_pair(fec, vmm), std::make_pair(detector, plane)));
          /*
          pFecChip_DetectorPlane_Labels.emplace(
              std::make_pair(std::make_pair(fec, vmm),
                             std::make_pair(labelDetector, labelPlane)));
          */
        }
      }
    } catch (const std::exception &exc) {
      throw std::runtime_error("Invalid json while parsing geometry file.");
    }
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
    float val = v[0];
    for (int n = 1; n < pDets.size(); n++) {
      v.push_back(val);
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
