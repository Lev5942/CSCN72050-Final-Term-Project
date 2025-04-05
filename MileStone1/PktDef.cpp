// File: PktDef.cpp

#include "PktDef.h"
#include <cstdlib>

PktDef::PktDef() {
    pkt.H.PktCount = 0;
    pkt.H.Flags = 0;
    pkt.H.Length = 0;
    pkt.Data = nullptr;
    pkt.CRC = 0;
    RawBuffer = nullptr;
}

PktDef::PktDef(char* rawData) {
    pkt.H.PktCount = (rawData[0] << 8) | rawData[1];
    pkt.H.Flags = rawData[2];
    pkt.H.Length = rawData[3];

    int dataLen = pkt.H.Length - HEADERSIZE - 1;
    if (dataLen > 0) {
        pkt.Data = new char[dataLen];
        memcpy(pkt.Data, &rawData[4], dataLen);
    }
    else {
        pkt.Data = nullptr;
    }

    pkt.CRC = rawData[pkt.H.Length - 1];
    RawBuffer = nullptr;
}

void PktDef::SetCmd(CmdType cmd) {
    pkt.H.Flags &= 0x0F;
    switch (cmd) {
    case DRIVE: pkt.H.Flags |= 0x80; break;
    case RESPONSE: pkt.H.Flags |= 0x40; break;
    case SLEEP: pkt.H.Flags |= 0x20; break;
    }
}

void PktDef::SetBodyData(char* body, int size) {
    if (pkt.Data) delete[] pkt.Data;
    pkt.Data = new char[size];
    memcpy(pkt.Data, body, size);
    pkt.H.Length = HEADERSIZE + size + 1;
}

void PktDef::SetPktCount(int count) {
    pkt.H.PktCount = static_cast<uint16_t>(count);
}

PktDef::CmdType PktDef::GetCmd() {
    if (pkt.H.Flags & 0x80) return DRIVE;
    if (pkt.H.Flags & 0x40) return RESPONSE;
    if (pkt.H.Flags & 0x20) return SLEEP;
    return RESPONSE;
}

bool PktDef::GetAck() {
    return pkt.H.Flags & 0x10;
}

int PktDef::GetLength() {
    return pkt.H.Length;
}

char* PktDef::GetBodyData() {
    return pkt.Data;
}

int PktDef::GetPktCount() {
    return pkt.H.PktCount;
}

bool PktDef::CheckCRC(char* buffer, int size) {
    int bitCount = 0;
    for (int i = 0; i < size - 1; i++) {
        bitCount += CountBitsSet(static_cast<uint8_t>(buffer[i]));
    }
    return (bitCount == static_cast<uint8_t>(buffer[size - 1]));
}

void PktDef::CalcCRC() {
    int bitCount = CalcPacketBitCount();
    pkt.CRC = static_cast<uint8_t>(bitCount);
}

char* PktDef::GenPacket() {
    if (RawBuffer) delete[] RawBuffer;

    RawBuffer = new char[pkt.H.Length];
    RawBuffer[0] = (pkt.H.PktCount >> 8) & 0xFF;
    RawBuffer[1] = pkt.H.PktCount & 0xFF;
    RawBuffer[2] = pkt.H.Flags;
    RawBuffer[3] = pkt.H.Length;

    int idx = 4;
    if (pkt.Data) {
        memcpy(&RawBuffer[idx], pkt.Data, pkt.H.Length - HEADERSIZE - 1);
        idx += pkt.H.Length - HEADERSIZE - 1;
    }

    CalcCRC();
    RawBuffer[idx] = pkt.CRC;

    return RawBuffer;
}

int PktDef::CountBitsSet(uint8_t byte) {
    int count = 0;
    while (byte) {
        count += byte & 1;
        byte >>= 1;
    }
    return count;
}

int PktDef::CalcPacketBitCount() {
    int bitCount = 0;
    bitCount += CountBitsSet(pkt.H.PktCount >> 8);
    bitCount += CountBitsSet(pkt.H.PktCount & 0xFF);
    bitCount += CountBitsSet(pkt.H.Flags);
    bitCount += CountBitsSet(pkt.H.Length);

    if (pkt.Data) {
        for (int i = 0; i < pkt.H.Length - HEADERSIZE - 1; ++i) {
            bitCount += CountBitsSet(pkt.Data[i]);
        }
    }

    return bitCount;
}
