// Copyright (C) 2021 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Parser for ESS readout of VMM3a data
///
/// Stat counters accumulate
//===----------------------------------------------------------------------===//
#include <iostream>
#include <log.h>
#include <parser/VMM3Parser.h>


// Assume we start after the Common PacketHeader
int VMM3Parser::parse(const char *Buffer, unsigned int Size) {
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

  VMM3Parser::VMM3Data * DataPtr = (struct VMM3Data *)Buffer;
  for (unsigned int i = 0; i < Size/DataLength; i++) {
    Stats.Readouts++;
   
    VMM3Parser::VMM3Data Readout = DataPtr[i];
    if (Readout.RingId > MaxRingId) {
      LOG(WARNING) << "Invalid RingId " << Readout.RingId << ", Max is " <<  MaxRingId;
      Stats.ErrorRing++;
      continue;
    }
    if ((Readout.FENId > MaxFENId))  {
      LOG(WARNING) << "Invalid FENId " << Readout.FENId << " (valid: 0 - " <<  MaxFENId << ")";
      Stats.ErrorFEN++;
      continue;
    }
  
    if (Readout.DataLength != DataLength)  {
    	LOG(WARNING) << "Invalid header length " << Readout.DataLength << 
    	" - must be " <<  DataLength << " bytes";
      Stats.ErrorDataLength++;
      continue;
    }
 
    if (Readout.TimeLow > MaxFracTimeCount)  {
     LOG(WARNING) << "Invalid TimeLO " << Readout.TimeLow << 
    	" (max is " <<  MaxFracTimeCount << ")";
      Stats.ErrorTimeFrac++;
      continue;
    }

    if (Readout.BC > MaxBCValue)  {
       LOG(WARNING) << "Invalid BC " << Readout.BC << 
    	" (max is " <<  MaxBCValue << ")";
      Stats.ErrorBC++;
      continue;
    }

    if ((Readout.OTADC & ADCMask) > MaxADCValue) {
      LOG(WARNING) << "Invalid ADC " << (Readout.OTADC & 0x7fff) << 
    	" (max is " <<  MaxADCValue << ")";
      Stats.ErrorADC++;
      continue;
    }

    // So far no checks for GEO and TDC

    if (Readout.VMM > MaxVMMValue) {
      LOG(WARNING) << "Invalid VMM " << Readout.VMM << 
    	" (max is " <<  MaxVMMValue << ")";
      Stats.ErrorVMM++;
      continue;
    }

    if (Readout.Channel > MaxChannelValue) {
      LOG(WARNING) << "Invalid Channel " << Readout.Channel << 
    	" (max is " <<  MaxChannelValue << ")";
      Stats.ErrorChannel++;
      continue;
    }


    // Validation done, increment stats for decoded parameters

    if (Readout.OTADC & OverThresholdMask) {
      Stats.OverThreshold++;
    }

    if ((Readout.GEO & 0x80) == 0) {
      Stats.DataReadouts++;
    } else {
      Stats.CalibReadouts++;
    }

    GoodReadouts++;
    Result.push_back(Readout);
  }

  return GoodReadouts;
}
