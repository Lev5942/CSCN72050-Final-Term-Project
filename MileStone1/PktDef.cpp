// File: PktDef.cpp

#include "PktDef.h"
#include <cstdlib>

// Initialize all packet fields to a safe default state
PktDef::PktDef() {
    pkt.H.PktCount = 0;
    pkt.H.Flags = 0;
    pkt.H.Length = 0;
    pkt.Data = nullptr;
    pkt.CRC = 0;
    RawBuffer = nullptr;
}

// Parse incoming raw buffer and populate Header
PktDef::PktDef(char* rawData) {
    pkt.H.PktCount = (rawData[0] << 8) | rawData[1];
    pkt.H.Flags = rawData[2];
    pkt.H.Length = rawData[3];

    // Compute body size (Length = header + body + CRC)
    int dataLen = pkt.H.Length - HEADERSIZE - 1;

    // If body exists, allocate and copy it
    if (dataLen > 0) {
        pkt.Data = new char[dataLen];
        memcpy(pkt.Data, &rawData[4], dataLen);
    }
    else {
        pkt.Data = nullptr;
    }

    // Last byte is CRC
    pkt.CRC = rawData[pkt.H.Length - 1];
    RawBuffer = nullptr;
}

// Clear existing command bits (keep padding bits untouched)
void PktDef::SetCmd(CmdType cmd) {
    pkt.H.Flags &= 0x0F;

    // Set the appropriate command bit (Drive = 0x80, Status = 0x40, Sleep = 0x20)
    switch (cmd) {
    case DRIVE: pkt.H.Flags |= 0x80; break;
    case RESPONSE: pkt.H.Flags |= 0x40; break;
    case SLEEP: pkt.H.Flags |= 0x20; break;
    }
}

void PktDef::SetBodyData(char* body, int size) {
    // Free previous body memory if needed
    if (pkt.Data) delete[] pkt.Data;

    // Store new body data
    pkt.Data = new char[size];
    memcpy(pkt.Data, body, size);

    // Calculate full packet size (header + body + CRC)
    pkt.H.Length = HEADERSIZE + size + 1;
}

void PktDef::SetPktCount(int count) {
    pkt.H.PktCount = static_cast<uint16_t>(count);
}

PktDef::CmdType PktDef::GetCmd() {

    // Return command type based on which bit is set in flags
    if (pkt.H.Flags & 0x80) return DRIVE;
    if (pkt.H.Flags & 0x40) return RESPONSE;
    if (pkt.H.Flags & 0x20) return SLEEP;

    // Default fallback
    return RESPONSE;
}

bool PktDef::GetAck() {

    // True if ACK bit (bit 4) is set
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

    // Count all '1' bits from bytes 0 to size-2 (exclude CRC)
    int bitCount = 0;
    for (int i = 0; i < size - 1; i++) {
        bitCount += CountBitsSet(static_cast<uint8_t>(buffer[i]));
    }

    // Match with actual CRC byte
    return (bitCount == static_cast<uint8_t>(buffer[size - 1]));
}

// Calculate and store CRC for this packet
void PktDef::CalcCRC() {
    int bitCount = CalcPacketBitCount();
    pkt.CRC = static_cast<uint8_t>(bitCount);
}

char* PktDef::GenPacket() {
    if (RawBuffer) delete[] RawBuffer;

    RawBuffer = new char[pkt.H.Length];

    // Serialize Header
    RawBuffer[0] = (pkt.H.PktCount >> 8) & 0xFF;
    RawBuffer[1] = pkt.H.PktCount & 0xFF;
    RawBuffer[2] = pkt.H.Flags;
    RawBuffer[3] = pkt.H.Length;

    // Serialize Body
    int idx = 4;
    if (pkt.Data) {
        memcpy(&RawBuffer[idx], pkt.Data, pkt.H.Length - HEADERSIZE - 1);
        idx += pkt.H.Length - HEADERSIZE - 1;
    }

    // Calculate and insert CRC
    CalcCRC();
    RawBuffer[idx] = pkt.CRC;

    return RawBuffer;
}

// Return number of 1-bits in a byte
int PktDef::CountBitsSet(uint8_t byte) {
    int count = 0;
    while (byte) {
        count += byte & 1;
        byte >>= 1;
    }
    return count;
}

// Sum total 1-bits in all relevant fields
int PktDef::CalcPacketBitCount() {
    int bitCount = 0;
    bitCount += CountBitsSet(pkt.H.PktCount >> 8);
    bitCount += CountBitsSet(pkt.H.PktCount & 0xFF);
    bitCount += CountBitsSet(pkt.H.Flags);
    bitCount += CountBitsSet(pkt.H.Length);

    // Count 1-bits in body
    if (pkt.Data) {
        for (int i = 0; i < pkt.H.Length - HEADERSIZE - 1; ++i) {
            bitCount += CountBitsSet(pkt.Data[i]);
        }
    }

    return bitCount;
}
