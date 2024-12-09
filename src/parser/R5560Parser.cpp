// Copyright (C) 2021 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Parser for ESS readout of R5560 data
///
/// Stat counters accumulate
//===----------------------------------------------------------------------===//
#include <iostream>
#include <parser/R5560Parser.h>
#include <parser/Trace.h>

#undef TRC_LEVEL
#define TRC_LEVEL TRC_L_DEB

// Assume we start after the Common PacketHeader
int R5560Parser::parse(const char *Buffer, unsigned int Size) {
  Result.clear();
  uint32_t GoodReadouts{0};

  if (Buffer == nullptr) {
    Stats.ErrorSize++;
    XTRACE(DATA, WAR, "Invalid data pointer");
    std::cout << "Invalid data pointer" << std::endl;
    return GoodReadouts;
  }

  if (Size % DataLength != 0) {
    Stats.ErrorSize++;
    XTRACE(DATA, WAR, "Invalid data length - %d should be multiple of %d", Size,
           DataLength);
    return GoodReadouts;
  }

  R5560Parser::R5560Data *DataPtr = (struct R5560Data *)Buffer;

  for (unsigned int i = 0; i < Size / DataLength; i++) {
    Stats.Readouts++;

    R5560Parser::R5560Data Readout = DataPtr[i];
    if (Readout.RingId > MaxRingId) {
      XTRACE(DATA, WAR, "Invalid RingId %d (Max is %d)", Readout.RingId,
             MaxRingId);
      Stats.ErrorRing++;
      continue;
    }
    if ((Readout.FENId > MaxFENId)) {
      XTRACE(DATA, WAR, "Invalid FENId %d (valid: 0 - %d)", Readout.FENId,
             MaxFENId);
      Stats.ErrorFEN++;
      continue;
    }

    if (Readout.DataLength != DataLength) {
      XTRACE(DATA, WAR, "Invalid header length %d - must be %d bytes",
             Readout.DataLength, DataLength);
      Stats.ErrorDataLength++;
      continue;
    }

    Stats.Readouts++;

    GoodReadouts++;
    Result.push_back(Readout);
  }

  return GoodReadouts;
}
