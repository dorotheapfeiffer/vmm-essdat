// Copyright (C) 2021 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief ESS I-BM readout parser
///
//===----------------------------------------------------------------------===//

#pragma once
#include <cinttypes>
#include <parser/ReadoutParser.h>
#include <vector>

struct IBMParserStats {
  int64_t ErrorSize{0};
  int64_t ErrorRing{0};
  int64_t ErrorFEN{0};
  int64_t ErrorDataLength{0};
  int64_t ErrorADC{0};
  int64_t Readouts{0};
};

class IBMParser {
public:
  const unsigned int MaxRingId{23}; // Physical rings
  const unsigned int MaxFENId{16};
  const unsigned int MaxReadoutsInPacket{500};

#define IBMDATASIZE 20

  struct IBMData {
    uint8_t RingId;
    uint8_t FENId;
    uint16_t DataLength;
    uint32_t TimeHigh;
    uint32_t TimeLow;
    uint32_t Type;
    uint32_t ADC;
    

  } __attribute__((packed));

  static_assert(sizeof(IBMParser::IBMData) == (IBMDATASIZE),
                "Wrong header size (update assert or check packing)");

  IBMParser() { Result.reserve(MaxReadoutsInPacket); };
  ~IBMParser(){};

  //
  int parse(const char *buffer, unsigned int size);

  // To be iterated over in processing thread
  std::vector<struct IBMData> Result;

  struct IBMParserStats Stats;

private:
  const uint16_t DataLength{20};
};
