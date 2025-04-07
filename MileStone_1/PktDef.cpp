#include "PktDef.h"
#include <cstring>

PktDef::PktDef() {
    packet.Data = nullptr;
    packet.CRC = 0;
    RawBuffer = nullptr;
    hasTelemetry = false;
}

PktDef::PktDef(char* raw) {
    RawBuffer = nullptr;
    hasTelemetry = false;

    packet.header.PktCount = (raw[0] << 8) | raw[1];
    packet.header.Drive = (raw[2] >> 7) & 0x1;
    packet.header.Status = (raw[2] >> 6) & 0x1;
    packet.header.Sleep = (raw[2] >> 5) & 0x1;
    packet.header.Ack = (raw[2] >> 4) & 0x1;
    packet.header.Padding = raw[2] & 0xF;
    packet.header.Length = raw[3];

    int bodyLen = packet.header.Length - HEADERSIZE;
    packet.Data = new char[bodyLen];
    memcpy(packet.Data, raw + 4, bodyLen);
    packet.CRC = raw[4 + bodyLen];

    if (packet.header.Status == 1 && bodyLen == 9) {
        telemetryData.LastPktCounter = (packet.Data[0] << 8) | packet.Data[1];
        telemetryData.CurrentGrade = (packet.Data[2] << 8) | packet.Data[3];
        telemetryData.HitCount = (packet.Data[4] << 8) | packet.Data[5];
        telemetryData.LastCmd = packet.Data[6];
        telemetryData.LastCmdValue = packet.Data[7];
        telemetryData.LastCmdSpeed = packet.Data[8];
        hasTelemetry = true;
    }
}

PktDef::~PktDef() {
    delete[] packet.Data;
    delete[] RawBuffer;
}

void PktDef::SetCmd(CmdType cmd) {
    packet.header.Drive = (cmd == DRIVE);
    packet.header.Sleep = (cmd == SLEEP);
    packet.header.Status = (cmd == RESPONSE);
}

void PktDef::SetBodyData(char* data, int size) {
    delete[] packet.Data;
    packet.Data = new char[size];
    memcpy(packet.Data, data, size);
    packet.header.Length = HEADERSIZE + size;
}

void PktDef::SetPktCount(int count) {
    packet.header.PktCount = count;
}

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

//bool PktDef::CheckCRC(char* buffer, int size) {
//    return CalcCRC(buffer, size - 1) == static_cast<unsigned char>(buffer[size - 1]);
//}

bool PktDef::CheckCRC(char* buffer, int size) {
    int expectedCRC = buffer[size - 1];

    // Extract bodyLen from buffer
    int bodyLen = buffer[3] - HEADERSIZE;

    // Temporarily decode header for CRC logic
    unsigned short pktCount = (buffer[0] << 8) | buffer[1];
    unsigned char flags = buffer[2];
    unsigned char len = buffer[3];

    int count = 0;
    for (int i = 0; i < 16; ++i) count += 1 & (pktCount >> i);
    count += 1 & ((flags >> 7) & 0x1); // Drive
    count += 1 & ((flags >> 6) & 0x1); // Status
    count += 1 & ((flags >> 5) & 0x1); // Sleep
    count += 1 & ((flags >> 4) & 0x1); // Ack
    for (int i = 0; i < 4; ++i) count += 1 & ((flags >> i) & 0x1); // Padding
    for (int i = 0; i < 8; ++i) count += 1 & (len >> i);
    for (int i = 0; i < bodyLen; ++i)
        for (int b = 0; b < 8; ++b)
            count += 1 & (buffer[4 + i] >> b);

    return expectedCRC == (count & 0xFF);
}


void PktDef::CalcCRC() {
    int count = 0;

    // PktCount: 16 bits
    for (int i = 0; i < 16; ++i) {
        count += 1 & (packet.header.PktCount >> i);
    }

    // Flags
    count += 1 & packet.header.Drive;
    count += 1 & packet.header.Status;
    count += 1 & packet.header.Sleep;
    count += 1 & packet.header.Ack;

    // Padding (4 bits)
    for (int i = 0; i < 4; ++i) {
        count += 1 & (packet.header.Padding >> i);
    }

    // Length: 8 bits
    for (int i = 0; i < 8; ++i) {
        count += 1 & (packet.header.Length >> i);
    }

    // Body data bits
    int bodyLen = packet.header.Length - HEADERSIZE;
    if (packet.Data != nullptr) {
        for (int i = 0; i < bodyLen; ++i) {
            for (int b = 0; b < 8; ++b) {
                count += 1 & (packet.Data[i] >> b);
            }
        }
    }

    // Store CRC
    packet.CRC = static_cast<unsigned char>(count);
}



char* PktDef::GenPacket() {
    delete[] RawBuffer;
    int size = packet.header.Length;
    RawBuffer = new char[size + 1];

    // Serialize header
    RawBuffer[0] = (packet.header.PktCount >> 8) & 0xFF;
    RawBuffer[1] = packet.header.PktCount & 0xFF;
    RawBuffer[2] = (packet.header.Drive << 7) |
        (packet.header.Status << 6) |
        (packet.header.Sleep << 5) |
        (packet.header.Ack << 4) |
        (packet.header.Padding & 0x0F);
    RawBuffer[3] = packet.header.Length;

    // Copy body
    int bodyLen = packet.header.Length - HEADERSIZE;
    memcpy(RawBuffer + 4, packet.Data, bodyLen);

    // Calculate and store CRC
    CalcCRC();
    RawBuffer[4 + bodyLen] = packet.CRC;

    return RawBuffer;
}


bool PktDef::HasTelemetry() {
    return hasTelemetry;
}

PktDef::Telemetry PktDef::GetTelemetry() {
    return telemetryData;
}

void PktDef::SetTelemetryData(unsigned short int lastPkt, unsigned short int grade, unsigned short int hits,
    unsigned char cmd, unsigned char val, unsigned char spd) {
    telemetryData.LastPktCounter = lastPkt;
    telemetryData.CurrentGrade = grade;
    telemetryData.HitCount = hits;
    telemetryData.LastCmd = cmd;
    telemetryData.LastCmdValue = val;
    telemetryData.LastCmdSpeed = spd;
    hasTelemetry = true;

    packet.header.Status = 1;
    packet.header.Length = HEADERSIZE + 9;

    delete[] packet.Data;
    packet.Data = new char[9];
    packet.Data[0] = lastPkt >> 8;
    packet.Data[1] = lastPkt & 0xFF;
    packet.Data[2] = grade >> 8;
    packet.Data[3] = grade & 0xFF;
    packet.Data[4] = hits >> 8;
    packet.Data[5] = hits & 0xFF;
    packet.Data[6] = cmd;
    packet.Data[7] = val;
    packet.Data[8] = spd;
}
