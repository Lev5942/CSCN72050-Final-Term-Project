// File: PktDef.h

#pragma once
#include <cstdint>
#include <cstring>

class PktDef {
public:
    enum CmdType { DRIVE, SLEEP, RESPONSE };

    static const uint8_t FORWARD = 1;
    static const uint8_t BACKWARD = 2;
    static const uint8_t RIGHT = 3;
    static const uint8_t LEFT = 4;
    static const uint8_t HEADERSIZE = 4;

    struct Header {
        uint16_t PktCount;
        uint8_t Flags;
        uint8_t Length;
    };

    struct DriveBody {
        uint8_t Direction;
        uint8_t Duration;
        uint8_t Speed;
    };

    PktDef();
    PktDef(char* rawData);

    void SetCmd(CmdType cmd);
    void SetBodyData(char* body, int size);
    void SetPktCount(int count);

    CmdType GetCmd();
    bool GetAck();
    int GetLength();
    char* GetBodyData();
    int GetPktCount();

    bool CheckCRC(char* buffer, int size);
    void CalcCRC();
    char* GenPacket();

private:
    struct CmdPacket {
        Header H;
        char* Data;
        uint8_t CRC;
    } pkt;

    char* RawBuffer;

    int CountBitsSet(uint8_t byte);
    int CalcPacketBitCount();
};
