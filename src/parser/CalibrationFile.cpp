/* Copyright (C) 2018 European Spallation Source, ERIC. See LICENSE file */
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Class for handling calibration files for VMM asics
///
//===----------------------------------------------------------------------===//

//#include <parser/Log.h>
#include <fstream>
#include <parser/CalibrationFile.h>
#include "Configuration.h"
#include <log.h>
#include <nlohmann/json.hpp>


using json = nlohmann::json;

/// \brief load calibration from file
CalibrationFile::CalibrationFile(std::string jsonfile) : CalibrationFile() {
  corryvreckan::Log::setSection("CalibrationFile");
  if (jsonfile.empty()) {
    return;
  }

  std::ifstream t(jsonfile);
  std::string Jsonstring((std::istreambuf_iterator<char>(t)),
                         std::istreambuf_iterator<char>());
  if (!t.good()) {
    throw std::runtime_error(
        "CalibrationFile error - requested file unavailable.");
  }

  loadCalibration(Jsonstring);
}

/// \brief parse json string with calibration data
void CalibrationFile::loadCalibration(std::string jsonstring) {
  corryvreckan::Log::setSection("CalibrationFile");
  nlohmann::json Root;
  try {
    Root = nlohmann::json::parse(jsonstring);
  } catch (...) {
  	LOG(ERROR) << "Invalid Json in calibration file";
    throw std::runtime_error("Invalid Json in calibration file.");
  }

  try {
    auto VmmCals = Root["vmm_calibration"];
    for (auto &vmmcal : VmmCals) {
      auto ring = vmmcal["ring"].get<size_t>();
      auto fen = vmmcal["fen"].get<size_t>();
      auto vmm = vmmcal["vmm"].get<size_t>();
      auto adc_offsets = vmmcal["adc_offset"];
      auto adc_slopes = vmmcal["adc_slope"];
      auto time_offsets = vmmcal["time_offset"];
      auto time_slopes = vmmcal["time_slope"];
     

      if ((adc_offsets.size() > 0 && adc_offsets.size() != MAX_CH) or
          (adc_slopes.size() > 0 && adc_slopes.size() != MAX_CH) or
          (time_offsets.size() > 0 && time_offsets.size() != MAX_CH) or
          (time_slopes.size() > 0 && time_slopes.size() != MAX_CH) ) {
 	LOG(ERROR) << "Invalid array lengths in calibration file";
        throw std::runtime_error("Invalid array lengths in calibration file.");
      }

      for (size_t j = 0; j < MAX_CH; j++) {
        float adc_slope = 1;
        float adc_offset = 0;
        float time_slope = 1;
        float time_offset = 0;
        

        if (adc_offsets.size() == MAX_CH) {
          adc_offset = adc_offsets[j].get<float>();
        }
        if (adc_slopes.size() == MAX_CH) {
          adc_slope = adc_slopes[j].get<float>();
        }
        if (time_offsets.size() == MAX_CH) {
          time_offset = time_offsets[j].get<float>();
        }
        if (time_slopes.size() == MAX_CH) {
          time_slope = time_slopes[j].get<float>();
        }
       
        addCalibration(ring, fen, vmm, j, adc_offset, adc_slope, time_offset,
                       time_slope);
      }
    }
  } catch (const std::exception &exc) {
   	LOG(ERROR) << "Invalid Json in calibration file field";
    throw std::runtime_error("Invalid json in calibration file field.");
  }
}

void CalibrationFile::addCalibration(size_t ring, size_t fen, size_t vmm, size_t ch,
                                     float adc_offset, float adc_slope,
                                     float time_offset, float time_slope) {
  size_t theFenIndex =  static_cast<uint8_t>(ring / 2) * FENS_PER_RING + fen;
  if (theFenIndex >= Calibrations.size()) {
    Calibrations.resize(theFenIndex + 1);
  }

  auto &theFens = Calibrations[theFenIndex];
  if (vmm >= theFens.size()) {
    theFens.resize(vmm + 1);
  }

  auto &theVmms = theFens[vmm];
  if (ch >= theVmms.size()) {
    theVmms.resize(ch + 1);
  }
  theVmms[ch] = {adc_offset, adc_slope,  time_offset, time_slope};
}

const Calibration &CalibrationFile::getCalibration(size_t ring, size_t fen, size_t vmm,
                                                   size_t ch) const {
  size_t theFenIndex =  static_cast<uint8_t>(ring / 2) * FENS_PER_RING + fen;
  if (theFenIndex >= Calibrations.size()) {
    return NoCorr;
  }

  const auto &theFens = Calibrations[theFenIndex];
  if (vmm >= theFens.size()) {
    return NoCorr;
  }

  const auto &theVmm = theFens[vmm];
  if (ch >= theVmm.size()) {
    return NoCorr;
  }
  return theVmm[ch];

}

