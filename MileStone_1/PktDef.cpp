#include "PktDef.h"
#include <cstdlib>

// Default Constructor 
PktDef::PktDef() {
    packet.header = { 0 };
    packet.Data = nullptr;
    packet.CRC = 0;
    RawBuffer = nullptr;
}

// Constructor from Raw Data 
PktDef::PktDef(char* raw) {
    std::memset(&packet.header, 0, sizeof(Header));

    int offset = 0;

    // Read PktCount
    std::memcpy(&packet.header.PktCount, raw + offset, sizeof(packet.header.PktCount));
    offset += sizeof(packet.header.PktCount);

    // Read flags
    unsigned char flagsByte;
    std::memcpy(&flagsByte, raw + offset, sizeof(flagsByte));
    packet.header.Drive = (flagsByte >> 7) & 0x01;
    packet.header.Status = (flagsByte >> 6) & 0x01;
    packet.header.Sleep = (flagsByte >> 5) & 0x01;
    packet.header.Ack = (flagsByte >> 4) & 0x01;
    packet.header.Padding = flagsByte & 0x0F;
    offset += sizeof(flagsByte);

    // Read Length
    std::memcpy(&packet.header.Length, raw + offset, sizeof(packet.header.Length));
    offset += sizeof(packet.header.Length);

    // Body size
    int bodySize = packet.header.Length - HEADERSIZE;

    if (bodySize > 0) {
        packet.Data = new char[bodySize];
        std::memcpy(packet.Data, raw + offset, bodySize);
        offset += bodySize;
    }
    else {
        packet.Data = nullptr;
    }

    // Read CRC
    std::memcpy(&packet.CRC, raw + offset, sizeof(packet.CRC));

    RawBuffer = nullptr;
}


// Set Command Type 
void PktDef::SetCmd(CmdType cmd) {
    packet.header.Drive = 0;
    packet.header.Status = 0;
    packet.header.Sleep = 0;

    switch (cmd) {
    case DRIVE: packet.header.Drive = 1; break;
    case SLEEP: packet.header.Sleep = 1; break;
    case RESPONSE: packet.header.Status = 1; break;
    }
}

// Set Packet Count 
void PktDef::SetPktCount(int count) {
    packet.header.PktCount = count;
}

// Set Body Data 
void PktDef::SetBodyData(char* data, int size) {
    if (packet.Data) delete[] packet.Data;

    packet.Data = new char[size];
    std::memcpy(packet.Data, data, size);
    packet.header.Length = HEADERSIZE + size;
}

// Get Command Type 
PktDef::CmdType PktDef::GetCmd() {
    if (packet.header.Drive) return DRIVE;
    if (packet.header.Sleep) return SLEEP;
    return RESPONSE;
}

bool PktDef::GetAck() {
    return packet.header.Ack;
}

int PktDef::GetLength() {
    return packet.header.Length;
}

char* PktDef::GetBodyData() {
    return packet.Data;
}

int PktDef::GetPktCount() {
    return packet.header.PktCount;
}

// CRC 
void PktDef::CalcCRC() {
    unsigned char count = 0;
    int bodyLen = packet.header.Length - HEADERSIZE;

    // Count bits from Header
    unsigned char* rawHeader = reinterpret_cast<unsigned char*>(&packet.header);
    for (int i = 0; i < 5; ++i) {  // 2 bytes (PktCount) + 1 (flags) + 2 (Length)
        unsigned char byte = rawHeader[i];
        while (byte) {
            count += byte & 1;
            byte >>= 1;
        }
    }

    // Count bits from Data
    for (int i = 0; i < bodyLen; ++i) {
        unsigned char byte = static_cast<unsigned char>(packet.Data[i]);
        while (byte) {
            count += byte & 1;
            byte >>= 1;
        }
    }

    packet.CRC = count;
}


// Check CRC from buffer 
bool PktDef::CheckCRC(char* buffer, int size) {
    unsigned char count = 0;
    for (int i = 0; i < size - 1; ++i) {
        unsigned char byte = buffer[i];
        for (int b = 0; b < 8; ++b) {
            if (byte & (1 << b)) count++;
        }
    }
    return count == (unsigned char)buffer[size - 1];
}

// Generate Packet 
char* PktDef::GenPacket() {
    if (RawBuffer) {
        delete[] RawBuffer;
    }

    int totalSize = packet.header.Length;
    int bodySize = totalSize - HEADERSIZE;

    RawBuffer = new char[totalSize];
    std::memset(RawBuffer, 0, totalSize);

    int offset = 0;

    // Copy PktCount
    std::memcpy(RawBuffer + offset, &packet.header.PktCount, sizeof(packet.header.PktCount));
    offset += sizeof(packet.header.PktCount);

    // Combine flags into one byte
    unsigned char flagsByte = (packet.header.Drive << 7) |
        (packet.header.Status << 6) |
        (packet.header.Sleep << 5) |
        (packet.header.Ack << 4) |
        (packet.header.Padding & 0x0F);
    std::memcpy(RawBuffer + offset, &flagsByte, sizeof(flagsByte));
    offset += sizeof(flagsByte);

    // Copy Length
    std::memcpy(RawBuffer + offset, &packet.header.Length, sizeof(packet.header.Length));
    offset += sizeof(packet.header.Length);

    // Copy Data
    if (bodySize > 0 && packet.Data) {
        std::memcpy(RawBuffer + offset, packet.Data, bodySize);
        offset += bodySize;
    }

    // Calculate and append CRC
    CalcCRC();
    std::memcpy(RawBuffer + offset, &packet.CRC, sizeof(packet.CRC));

    return RawBuffer;
}
