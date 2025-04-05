#pragma once
#include <cstdint>
#include <cstring>

class PktDef {
public:
    // Supported command types
    enum CmdType { DRIVE, SLEEP, RESPONSE };

    // Drive direction constants
    static const uint8_t FORWARD = 1;
    static const uint8_t BACKWARD = 2;
    static const uint8_t RIGHT = 3;
    static const uint8_t LEFT = 4;

    // Number of bytes in header (PktCount + Flags + Length)
    static const uint8_t HEADERSIZE = 4;

    // Application protocol packet header
    struct Header {
        uint16_t PktCount;   // 2 bytes
        uint8_t Flags;       // 1 byte (bit-packed)
        uint8_t Length;      // 1 byte (total size)
    };

    // Drive-specific command body
    struct DriveBody {
        uint8_t Direction;
        uint8_t Duration;
        uint8_t Speed;
    };

    // Constructors
    PktDef();                 // Create empty packet
    PktDef(char* rawData);    // Parse from raw buffer

    // Mutators
    void SetCmd(CmdType cmd);                     // Set flag bits for command
    void SetBodyData(char* body, int size);       // Store packet body
    void SetPktCount(int count);                  // Set packet sequence #

    // Accessors
    CmdType GetCmd();         // Identify type by flags
    bool GetAck();            // True if ACK flag is set
    int GetLength();          // Full packet length
    char* GetBodyData();      // Pointer to packet body
    int GetPktCount();        // Return packet count

    // CRC
    bool CheckCRC(char* buffer, int size);  // Validate buffer CRC
    void CalcCRC();                         // Calculate CRC for this packet

    // Final serialized output
    char* GenPacket();                      // Build raw packet for sending

private:
    struct CmdPacket {
        Header H;
        char* Data;
        uint8_t CRC;
    } pkt;

    char* RawBuffer;   // Complete serialized buffer

    int CountBitsSet(uint8_t byte);     // Helper for CRC
    int CalcPacketBitCount();           // CRC: sum of all 1-bits
};
