// Copyright (C) 2025 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Parser for ESS readout of I-BM data
///
/// Stat counters accumulate
//===----------------------------------------------------------------------===//
#include <iostream>
#include <log.h>
#include <parser/IBMParser.h>

// Assume we start after the Common PacketHeader
int IBMParser::parse(const char *Buffer, unsigned int Size) {
  corryvreckan::Log::setSection("IBMParser");
  Result.clear();
  uint32_t GoodReadouts{0};

  if (Buffer == nullptr) {
    Stats.ErrorSize++;
    LOG(WARNING) << "Invalid data pointer";
    return GoodReadouts;
  }

  if (Size % DataLength != 0) {
    Stats.ErrorSize++;
    LOG(WARNING) << "Invalid data length - (" << Size << 
    ") should be multiple of " << DataLength;
    return GoodReadouts;
  }

  IBMParser::IBMData *DataPtr = (struct IBMData *)Buffer;

  for (unsigned int i = 0; i < Size / DataLength; i++) {
    Stats.Readouts++;

    IBMParser::IBMData Readout = DataPtr[i];
    if (Readout.RingId > MaxRingId) {
    	LOG(WARNING) << "Invalid RingId " << Readout.RingId << ", Max is " <<  MaxRingId;
      Stats.ErrorRing++;
      continue;
    }
    if ((Readout.FENId > MaxFENId)) {
        LOG(WARNING) << "Invalid FENId " << Readout.FENId << " (valid: 0 - " <<  MaxFENId << ")";
      Stats.ErrorFEN++;
      continue;
    }

    if (Readout.DataLength != DataLength) {
    	LOG(WARNING) << "Invalid header length " << Readout.DataLength << 
    	" - must be " <<  DataLength << " bytes";
      Stats.ErrorDataLength++;
      continue;
    }

    Stats.Readouts++;

    GoodReadouts++;
    Result.push_back(Readout);
  }

  return GoodReadouts;
}
