// Copyright (C) 2017-2020 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief ESS readout data parser implementation
///
//===----------------------------------------------------------------------===//
//#include <iostream>
#include <arpa/inet.h>
#include <cstring>
#include <log.h>
#include <parser/ReadoutParser.h>


ReadoutParser::ReadoutParser() {
  std::memset(NextSeqNum, 0, sizeof(NextSeqNum));
}

int ReadoutParser::validate(const char *Buffer, uint32_t Size,
                            uint8_t ExpectedType) {
  std::memset(&Packet, 0, sizeof(Packet));
  corryvreckan::Log::setSection("ReadoutParser");
  if (Buffer == nullptr or Size == 0) {
    LOG(WARNING) << "No buffer specified!";
    Stats.ErrorBuffer++;
    return -ReadoutParser::EBUFFER;
  }

  if ((Size < MinDataSize) || (Size > MaxUdpDataSize)) {
    LOG(WARNING) << "Invalid data size " << Size;
    Stats.ErrorSize++;
    return -ReadoutParser::ESIZE;
  }

  uint32_t Version = htons(*(uint16_t *)(Buffer));
  if ((Version >> 8) != 0) {
    LOG(WARNING) << "Padding is wrong (should be 0)";
    Stats.ErrorPad++;
    return -ReadoutParser::EHEADER;
  }

  if ((Version & 0xff) > 0x01) { //
    LOG(WARNING) << "Invalid version: expected 0, got " << static_cast<int>(Version & 0xff);
    Stats.ErrorVersion++;
    return -ReadoutParser::EHEADER;
  }

  // Check cookie
  uint32_t SwappedCookie = (*(uint32_t *)(Buffer + 2)) & 0xffffff;
  if (SwappedCookie != 0x535345) {
    LOG(WARNING) << "Wrong Cookie (" << SwappedCookie << "), 0x535345 or 'ESS' expected";
    Stats.ErrorCookie++;
    return -ReadoutParser::EHEADER;
  }

  uint8_t Type = 0;

  // Packet is ESS readout version 0, now we can add more header size checks
  if ((Version & 0xff) == 0x00) {
    Packet.version = 0;
    if (Size < sizeof(PacketHeaderV0)) {
      LOG(WARNING) << "Invalid data size for version 0 (" << Size << ")";
      Stats.ErrorSize++;
      return -ReadoutParser::ESIZE;
    }
    // It is safe to cast packet header v0 struct to data
    Packet.HeaderPtr0 = (PacketHeaderV0 *)Buffer;

#ifndef OMITSIZECHECK
    if (Size != Packet.HeaderPtr0->TotalLength or
        Packet.HeaderPtr0->TotalLength < sizeof(PacketHeaderV0)) {
      LOG(WARNING) << "Data length mismatch, expected " << 
             Packet.HeaderPtr0->TotalLength << ", got " <<  Size;
      Stats.ErrorSize++;
      return -ReadoutParser::ESIZE;
    }
#endif
    Type = Packet.HeaderPtr0->CookieAndType >> 24;

    if (Packet.HeaderPtr0->OutputQueue >= MaxOutputQueues) {
      LOG(WARNING) << "Output queue " << 
             Packet.HeaderPtr0->OutputQueue << " exceeds max size: " 
             << MaxOutputQueues;
      Stats.ErrorOutputQueue++;
      return -ReadoutParser::EHEADER;
    }

    if (NextSeqNum[Packet.HeaderPtr0->OutputQueue] !=
        Packet.HeaderPtr0->SeqNum) {
      LOG(TRACE) << "Bad sequence number for OQ "
      << Packet.HeaderPtr0->OutputQueue << ", expected " <<
             NextSeqNum[Packet.HeaderPtr0->OutputQueue] << ", got " <<
             Packet.HeaderPtr0->SeqNum;
      Stats.ErrorSeqNum++;
      NextSeqNum[Packet.HeaderPtr0->OutputQueue] = Packet.HeaderPtr0->SeqNum;
    }

    NextSeqNum[Packet.HeaderPtr0->OutputQueue]++;
    Packet.DataPtr = (char *)(Buffer + sizeof(PacketHeaderV0));
    Packet.DataLength = Packet.HeaderPtr0->TotalLength - sizeof(PacketHeaderV0);

    // Check time values
    if (Packet.HeaderPtr0->PulseLow > MaxFracTimeCount) {
       LOG(WARNING) << "Pulse time low (" <<Packet.HeaderPtr0->PulseLow
       << ") exceeds max cycle count (" << MaxFracTimeCount
       << ")";
      Stats.ErrorTimeFrac++;
      return -ReadoutParser::EHEADER;
    }

    if (Packet.HeaderPtr0->PrevPulseLow > MaxFracTimeCount) {
      LOG(WARNING) << "Previous Pulse time low (" <<Packet.HeaderPtr0->PrevPulseLow
       << ") exceeds max cycle count (" << MaxFracTimeCount
       << ")";
      Stats.ErrorTimeFrac++;
      return -ReadoutParser::EHEADER;
    }

    if (Packet.HeaderPtr0->TotalLength ==
        sizeof(ReadoutParser::PacketHeaderV0)) {
      LOG(TRACE) << "Heartbeat packet (pulse time only)";
      Stats.HeartBeats++;
    }

  }
  // Version 1 of RMM data format
  else if ((Version & 0xff) == 0x01) {
    Packet.version = 1;
    if (Size < sizeof(PacketHeaderV1)) {
      LOG(WARNING) << "Invalid data size for version 1 (" << Size << ")";
    
      Stats.ErrorSize++;
      return -ReadoutParser::ESIZE;
    }
    // It is safe to cast packet header v0 struct to data
    Packet.HeaderPtr1 = (PacketHeaderV1 *)Buffer;

#ifndef OMITSIZECHECK
    if (Size != Packet.HeaderPtr1->TotalLength or
        Packet.HeaderPtr1->TotalLength < sizeof(PacketHeaderV1)) {
            LOG(WARNING) << "Data length mismatch, expected " << 
             Packet.HeaderPtr1->TotalLength << ", got " <<  Size;
      Stats.ErrorSize++;
      return -ReadoutParser::ESIZE;
    }
#endif
    Type = Packet.HeaderPtr1->CookieAndType >> 24;

    if (Packet.HeaderPtr1->OutputQueue >= MaxOutputQueues) {
      LOG(WARNING) << "Output queue " << 
             Packet.HeaderPtr1->OutputQueue << " exceeds max size: " 
             << MaxOutputQueues;
      Stats.ErrorOutputQueue++;
      return -ReadoutParser::EHEADER;
    }

    if (NextSeqNum[Packet.HeaderPtr1->OutputQueue] !=
        Packet.HeaderPtr1->SeqNum) {
        LOG(TRACE) << "Bad sequence number for OQ "
      << Packet.HeaderPtr1->OutputQueue << ", expected " <<
             NextSeqNum[Packet.HeaderPtr1->OutputQueue] << ", got " <<
             Packet.HeaderPtr1->SeqNum;
      Stats.ErrorSeqNum++;
      NextSeqNum[Packet.HeaderPtr1->OutputQueue] = Packet.HeaderPtr1->SeqNum;
    }

    NextSeqNum[Packet.HeaderPtr1->OutputQueue]++;
    Packet.DataPtr = (char *)(Buffer + sizeof(PacketHeaderV1));
    Packet.DataLength = Packet.HeaderPtr1->TotalLength - sizeof(PacketHeaderV1);

    // Check time values
    if (Packet.HeaderPtr1->PulseLow > MaxFracTimeCount) {
      LOG(WARNING) << "Pulse time low (" <<Packet.HeaderPtr1->PulseLow
       << ") exceeds max cycle count (" << MaxFracTimeCount
       << ")";
      Stats.ErrorTimeFrac++;
      return -ReadoutParser::EHEADER;
    }

    if (Packet.HeaderPtr1->PrevPulseLow > MaxFracTimeCount) {
      LOG(WARNING) << "Previous Pulse time low (" <<Packet.HeaderPtr1->PrevPulseLow
       << ") exceeds max cycle count (" << MaxFracTimeCount
       << ")";       
      Stats.ErrorTimeFrac++;
      return -ReadoutParser::EHEADER;
    }

    if (Packet.HeaderPtr1->TotalLength ==
        sizeof(ReadoutParser::PacketHeaderV1)) {
      LOG(TRACE) << "Heartbeat packet (pulse time only)";
      Stats.HeartBeats++;
    }
  }

  return ReadoutParser::OK;
}
