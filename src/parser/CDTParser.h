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

struct CDTParserStats {
  int64_t ErrorSize{0};
  int64_t ErrorRing{0};
  int64_t ErrorFEN{0};
  int64_t ErrorDataLength{0};
  int64_t ErrorADC{0};
  int64_t Readouts{0};
};

class CDTParser {
public:
  const unsigned int MaxRingId{23}; // Physical rings
  const unsigned int MaxFENId{16};
  const unsigned int MaxReadoutsInPacket{500};

#define CDTDATASIZE 16

  struct CDTData {
    uint8_t RingId;
    uint8_t FENId;
    uint16_t DataLength;
    uint32_t TimeHigh;
    uint32_t TimeLow;
    uint8_t OM;
    uint8_t UID;
    uint8_t Cathode;
    uint8_t Anode;
  } __attribute__((packed));

  static_assert(sizeof(CDTParser::CDTData) == (CDTDATASIZE),
                "Wrong header size (update assert or check packing)");

  CDTParser() { Result.reserve(MaxReadoutsInPacket); };
  ~CDTParser(){};

  //
  int64_t parse(const char *buffer, unsigned int size);

  // To be iterated over in processing thread
  std::vector<struct CDTData> Result;

  struct CDTParserStats Stats;

private:
  const uint16_t DataLength{CDTDATASIZE};
};
