// Copyright (C) 2021 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief ESS R5560 readout parser
///
//===----------------------------------------------------------------------===//

#pragma once
#include <cinttypes>
#include <parser/ReadoutParser.h>
#include <vector>

struct R5560ParserStats {
  int64_t ErrorSize{0};
  int64_t ErrorRing{0};
  int64_t ErrorFEN{0};
  int64_t ErrorDataLength{0};
  int64_t ErrorGroup{0};
  int64_t ErrorAmplitudeA{0};
  int64_t ErrorAmplitudeB{0};
  int64_t ErrorAmplitudeC{0};
  int64_t ErrorAmplitudeD{0};
  int64_t Readouts{0};
};

class R5560Parser {
public:
  const unsigned int MaxRingId{23}; // Physical rings
  const unsigned int MaxFENId{16};
  const unsigned int MaxReadoutsInPacket{500};

#define R5560DATASIZE 24

  struct R5560Data {
    uint8_t RingId;
    uint8_t FENId;
    uint16_t DataLength;
    uint32_t TimeHigh;
    uint32_t TimeLow;
    uint8_t OM;
    uint8_t Group;
    uint16_t Counter;
    uint16_t AmplitudeA;
    uint16_t AmplitudeB;
    uint16_t AmplitudeC;
    uint16_t AmplitudeD;

  } __attribute__((packed));

  static_assert(sizeof(R5560Parser::R5560Data) == (R5560DATASIZE),
                "Wrong header size (update assert or check packing)");

  R5560Parser() { Result.reserve(MaxReadoutsInPacket); };
  ~R5560Parser(){};

  //
  int parse(const char *buffer, unsigned int size);

  // To be iterated over in processing thread
  std::vector<struct R5560Data> Result;

  struct R5560ParserStats Stats;

private:
  const uint16_t DataLength{24};
};
