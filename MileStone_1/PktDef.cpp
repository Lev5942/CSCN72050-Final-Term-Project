#include "PktDef.h"
#include <cstring>

// Default constructor: initialize pointers and flags
PktDef::PktDef() {
    packet.Data = nullptr;      // No body data yet
    packet.CRC = 0;             // CRC initialized to 0
    RawBuffer = nullptr;        // Serialized packet buffer not allocated yet
    hasTelemetry = false;       // No telemetry yet
}

// Constructor that parses a received raw buffer into a packet
//PktDef::PktDef(char* raw) {
//    RawBuffer = nullptr;
//    hasTelemetry = false;
//
//    // Deserialize header
//    packet.header.PktCount = (raw[0] << 8) | raw[1]; // 2 bytes: big-endian
//    packet.header.Drive = (raw[2] >> 7) & 0x1;        // Flag bits
//    packet.header.Status = (raw[2] >> 6) & 0x1;
//    packet.header.Sleep = (raw[2] >> 5) & 0x1;
//    packet.header.Ack = (raw[2] >> 4) & 0x1;
//    packet.header.Padding = raw[2] & 0xF;             // Lower 4 bits
//    packet.header.Length = raw[3];                    // Length byte
//
//    int bodyLen = packet.header.Length - HEADERSIZE;
//
//    // Copy body data (if any)
//    packet.Data = new char[bodyLen];
//    memcpy(packet.Data, raw + 4, bodyLen);
//
//    // Last byte is CRC
//    packet.CRC = raw[4 + bodyLen];
//
//    // If this is a telemetry response with a body of 9 bytes, decode it
//    if (packet.header.Status == 1 && bodyLen == 9) {
//        telemetryData.LastPktCounter = (packet.Data[0] << 8) | packet.Data[1];
//        telemetryData.CurrentGrade = (packet.Data[2] << 8) | packet.Data[3];
//        telemetryData.HitCount = (packet.Data[4] << 8) | packet.Data[5];
//        telemetryData.LastCmd = packet.Data[6];
//        telemetryData.LastCmdValue = packet.Data[7];
//        telemetryData.LastCmdSpeed = packet.Data[8];
//        hasTelemetry = true;
//    }
//}

PktDef::PktDef(char* raw) {
    RawBuffer = nullptr;
    hasTelemetry = false;

    // Deserialize header
    packet.header.PktCount = (raw[0] << 8) | raw[1]; // 2 bytes: big-endian
    packet.header.Drive = (raw[2] >> 7) & 0x1;        // Flag bits
    packet.header.Status = (raw[2] >> 6) & 0x1;
    packet.header.Sleep = (raw[2] >> 5) & 0x1;
    packet.header.Ack = (raw[2] >> 4) & 0x1;
    packet.header.Padding = raw[2] & 0xF;             // Lower 4 bits
    packet.header.Length = raw[3];                    // Length byte

    int bodyLen = packet.header.Length - HEADERSIZE;  // Adjust for new header size

    // Copy body data (if any)
    packet.Data = new char[bodyLen];
    memcpy(packet.Data, raw + 4, bodyLen);

    // Last byte is CRC
    packet.CRC = raw[4 + bodyLen];

    // If this is a telemetry response with a body of 9 bytes, decode it
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


// Destructor: free dynamically allocated memory
PktDef::~PktDef() {
    delete[] packet.Data;
    delete[] RawBuffer;
}

// Set which command this packet represents
void PktDef::SetCmd(CmdType cmd) {
    packet.header.Drive = (cmd == DRIVE);
    packet.header.Sleep = (cmd == SLEEP);
    packet.header.Status = (cmd == RESPONSE);
}

// Set body data and update length accordingly
void PktDef::SetBodyData(char* data, int size) {
    delete[] packet.Data;
    packet.Data = new char[size];
    memcpy(packet.Data, data, size);
    packet.header.Length = HEADERSIZE + size;
}

// Set packet counter value
void PktDef::SetPktCount(int count) {
    packet.header.PktCount = count;
}

// Get which command this packet represents
PktDef::CmdType PktDef::GetCmd() {
    if (packet.header.Drive) return DRIVE;
    if (packet.header.Sleep) return SLEEP;
    return RESPONSE;
}

// Get Ack flag
bool PktDef::GetAck() {
    return packet.header.Ack;
}

// Get full packet length (header + body + CRC)
int PktDef::GetLength() {
    return packet.header.Length;
}

// Get pointer to packet body data
char* PktDef::GetBodyData() {
    return packet.Data;
}

// Get packet counter value
int PktDef::GetPktCount() {
    return packet.header.PktCount;
}

// Check CRC using the specified algorithm
bool PktDef::CheckCRC(char* buffer, int size) {
    int expectedCRC = buffer[size - 1];
    int bodyLen = buffer[3] - HEADERSIZE;

    // Extract header info to count bits
    unsigned short pktCount = (buffer[0] << 8) | buffer[1];
    unsigned char flags = buffer[2];
    unsigned char len = buffer[3];

    int count = 0;
    // Count 1-bits in each field
    for (int i = 0; i < 16; ++i) count += 1 & (pktCount >> i);
    count += 1 & ((flags >> 7) & 0x1); // Drive
    count += 1 & ((flags >> 6) & 0x1); // Status
    count += 1 & ((flags >> 5) & 0x1); // Sleep
    count += 1 & ((flags >> 4) & 0x1); // Ack
    for (int i = 0; i < 4; ++i) count += 1 & ((flags >> i) & 0x1); // Padding
    for (int i = 0; i < 8; ++i) count += 1 & (len >> i);

    // Count 1-bits in body data
    for (int i = 0; i < bodyLen; ++i)
        for (int b = 0; b < 8; ++b)
            count += 1 & (buffer[4 + i] >> b);

    return expectedCRC == (count & 0xFF);
}

//// Calculate CRC using bit-counting algorithm and store it
//void PktDef::CalcCRC() {
//    int count = 0;
//
//    // Count 1-bits in PktCount (16 bits)
//    for (int i = 0; i < 16; ++i) {
//        count += 1 & (packet.header.PktCount >> i);
//    }
//
//    // Flags
//    count += 1 & packet.header.Drive;
//    count += 1 & packet.header.Status;
//    count += 1 & packet.header.Sleep;
//    count += 1 & packet.header.Ack;
//
//    // Padding (4 bits)
//    for (int i = 0; i < 4; ++i) {
//        count += 1 & (packet.header.Padding >> i);
//    }
//
//    // Length (8 bits)
//    for (int i = 0; i < 8; ++i) {
//        count += 1 & (packet.header.Length >> i);
//    }
//
//    // Count 1-bits in body data
//    int bodyLen = packet.header.Length - HEADERSIZE;
//    if (packet.Data != nullptr) {
//        for (int i = 0; i < bodyLen; ++i) {
//            for (int b = 0; b < 8; ++b) {
//                count += 1 & (packet.Data[i] >> b);
//            }
//        }
//    }
//
//    // Store CRC as 8-bit value
//    packet.CRC = static_cast<unsigned char>(count);
//}

//void PktDef::CalcCRC() {
//    // Set CRC to 0 before calculation
//    packet.CRC = 0;
//
//    // Create a temporary buffer to simulate the packet layout for CRC calculation
//    int totalLength = packet.header.Length;
//    std::unique_ptr<char[]> tempBuffer(new char[totalLength]);
//
//    // Copy header
//    std::memcpy(tempBuffer.get(), &packet.header, sizeof(packet.header));
//
//    // Copy data if any
//    if (packet.Data != nullptr && totalLength > sizeof(packet.header) + 1) {
//        std::memcpy(tempBuffer.get() + sizeof(packet.header), packet.Data, totalLength - sizeof(packet.header) - 1);
//    }
//
//    // Calculate XOR-based CRC
//    unsigned char result = 0;
//    for (int i = 0; i < totalLength - 1; ++i) {  // exclude CRC itself
//        result ^= tempBuffer[i];
//    }
//
//    packet.CRC = result;
//}

void PktDef::CalcCRC() {
    // Set CRC to 0 before calculation
    packet.CRC = 0;

    // Total packet length = header + body + CRC
    int totalLength = packet.header.Length;

    // Allocate temporary buffer for header + body (exclude CRC initially)
    std::unique_ptr<char[]> tempBuffer(new char[totalLength - 1]);

    // Manually serialize the header into tempBuffer[0..3]
    tempBuffer[0] = (packet.header.PktCount >> 8) & 0xFF; // High byte
    tempBuffer[1] = packet.header.PktCount & 0xFF;        // Low byte
    tempBuffer[2] = (packet.header.Drive << 7) | (packet.header.Status << 6) |
        (packet.header.Sleep << 5) | (packet.header.Ack << 4) |
        (packet.header.Padding & 0x0F);
    tempBuffer[3] = packet.header.Length;

    // Copy body if any (Length includes header + body)
    int bodyLen = totalLength - HEADERSIZE;
    if (packet.Data && bodyLen > 0) {
        std::memcpy(tempBuffer.get() + HEADERSIZE, packet.Data, bodyLen);
    }

    // XOR all bytes in tempBuffer
    unsigned char crc = 0;
    for (int i = 0; i < totalLength - 1; ++i) {
        crc ^= tempBuffer[i];
    }

    // Store CRC
    packet.CRC = crc;
}



//// Generate raw buffer to send over network
//char* PktDef::GenPacket() {
//    delete[] RawBuffer;
//    int size = packet.header.Length;
//    RawBuffer = new char[size + 1]; // Allocate memory for header + body + CRC
//
//    // Serialize header fields
//    RawBuffer[0] = (packet.header.PktCount >> 8) & 0xFF;
//    RawBuffer[1] = packet.header.PktCount & 0xFF;
//    RawBuffer[2] = (packet.header.Drive << 7) |
//        (packet.header.Status << 6) |
//        (packet.header.Sleep << 5) |
//        (packet.header.Ack << 4) |
//        (packet.header.Padding & 0x0F);
//    RawBuffer[3] = packet.header.Length;
//
//    // Copy body (if any)
//    int bodyLen = packet.header.Length - HEADERSIZE;
//    memcpy(RawBuffer + 4, packet.Data, bodyLen);
//
//    // Append CRC at end
//    CalcCRC();
//    RawBuffer[4 + bodyLen] = packet.CRC;
//
//    return RawBuffer;
//}

//char* PktDef::GenPacket() {
//    // Total size = header length + 1 byte for CRC
//    int totalSize = packet.header.Length + 1;
//
//    // Cleanup any previously allocated buffer
//    if (RawBuffer) {
//        delete[] RawBuffer;
//    }
//
//    RawBuffer = new char[totalSize];
//
//    // Copy PktCount (2 bytes)
//    memcpy(RawBuffer, &packet.header.PktCount, 2);
//
//    // Pack command flags into 1 byte
//    unsigned char flags = 0;
//    flags |= (packet.header.Status & 0x01) << 0;
//    flags |= (packet.header.Drive & 0x01) << 1;
//    flags |= (packet.header.Sleep & 0x01) << 2;
//    flags |= (packet.header.Ack & 0x01) << 3;
//    flags |= (packet.header.Padding & 0x0F) << 4;
//    RawBuffer[2] = flags;
//
//    // Copy Length (1 byte)
//    RawBuffer[3] = packet.header.Length;
//
//    // Copy Body (if any)
//    if (packet.header.Length > HEADERSIZE && packet.Data != nullptr) {
//        memcpy(RawBuffer + HEADERSIZE, packet.Data, packet.header.Length - HEADERSIZE);
//    }
//
//    // Calculate CRC over [0..Length-1]
//    unsigned char crc = 0;
//    for (int i = 0; i < packet.header.Length; i++) {
//        crc ^= RawBuffer[i];
//    }
//
//    packet.CRC = crc;
//    RawBuffer[packet.header.Length] = packet.CRC;
//
//    return RawBuffer;
//}

//char* PktDef::GenPacket() {
//    // Total size = header length + 1 byte for CRC
//    int totalSize = packet.header.Length + 1;
//
//    // Cleanup any previously allocated buffer
//    delete[] RawBuffer;
//    RawBuffer = new char[totalSize];
//
//    // Serialize header fields
//    RawBuffer[0] = (packet.header.PktCount >> 8) & 0xFF;
//    RawBuffer[1] = packet.header.PktCount & 0xFF;
//    RawBuffer[2] = (packet.header.Drive << 7) |
//        (packet.header.Status << 6) |
//        (packet.header.Sleep << 5) |
//        (packet.header.Ack << 4) |
//        (packet.header.Padding & 0x0F);
//    RawBuffer[3] = packet.header.Length;
//
//    // Copy body (if any)
//    int bodyLen = packet.header.Length - HEADERSIZE;
//    if (bodyLen > 0 && packet.Data != nullptr) {
//        memcpy(RawBuffer + 4, packet.Data, bodyLen);
//    }
//
//    // Calculate CRC using bit-counting algorithm
//    CalcCRC();
//
//    // Append CRC
//    RawBuffer[4 + bodyLen] = packet.CRC;
//
//    return RawBuffer;
//}

char* PktDef::GenPacket() {
    // Cleanup any previously allocated buffer
    delete[] RawBuffer;

    int bodyLen = packet.header.Length - HEADERSIZE;
    int totalSize = packet.header.Length + 1; // +1 for CRC

    RawBuffer = new char[totalSize];

    // Serialize header
    RawBuffer[0] = (packet.header.PktCount >> 8) & 0xFF; // High byte
    RawBuffer[1] = packet.header.PktCount & 0xFF;        // Low byte
    RawBuffer[2] = (packet.header.Drive << 7) |
        (packet.header.Status << 6) |
        (packet.header.Sleep << 5) |
        (packet.header.Ack << 4) |
        (packet.header.Padding & 0x0F);
    RawBuffer[3] = packet.header.Length;

    // Serialize body data (if any)
    if (packet.Data != nullptr && bodyLen > 0) {
        memcpy(RawBuffer + 4, packet.Data, bodyLen);
    }

    // Calculate and append CRC
    CalcCRC(); // Updates packet.CRC
    RawBuffer[4 + bodyLen] = packet.CRC;

    return RawBuffer;
}



// Check if telemetry data is present in this packet
bool PktDef::HasTelemetry() {
    return hasTelemetry;
}

// Return telemetry data
PktDef::Telemetry PktDef::GetTelemetry() {
    return telemetryData;
}

// Manually set telemetry values and serialize them into packet
void PktDef::SetTelemetryData(unsigned short int lastPkt, unsigned short int grade, unsigned short int hits,
    unsigned char cmd, unsigned char val, unsigned char spd) {

    telemetryData.LastPktCounter = lastPkt;
    telemetryData.CurrentGrade = grade;
    telemetryData.HitCount = hits;
    telemetryData.LastCmd = cmd;
    telemetryData.LastCmdValue = val;
    telemetryData.LastCmdSpeed = spd;
    hasTelemetry = true;

    packet.header.Status = 1;               // Mark this as telemetry/status response
    packet.header.Length = HEADERSIZE + 9;  // Header + 9 bytes of telemetry

    // Create body from telemetry values
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
