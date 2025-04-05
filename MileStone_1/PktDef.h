#pragma once

#ifndef PKTDEF_H
#define PKTDEF_H

#include <memory>
#include <iostream>
#include <fstream>
#include <cstring>

class PktDef {
public:
    // Constants 
    static const unsigned char FORWARD = 1;
    static const unsigned char BACKWARD = 2;
    static const unsigned char RIGHT = 3;
    static const unsigned char LEFT = 4;
    static const int HEADERSIZE = 6;  // 2 (PktCount) + 1 (flags+pad) + 2 (Length) + 1 (CRC) = 6

    // Enums 
    enum CmdType { DRIVE, SLEEP, RESPONSE };

    // Structs 
    struct Header
    {
        unsigned short int PktCount;
        unsigned char Status : 1;
        unsigned char Drive;
        unsigned char Sleep : 1;
        unsigned char Ack : 1;
        unsigned char Padding : 4;
        unsigned char Length : 8;
    };

    struct Drivebody
    {
        unsigned char Direction;
        unsigned char Duration;
        unsigned char Speed;
    };

private:
    struct CmdPacket {
        Header header;
        char* Data;
        unsigned char CRC;
    };

    CmdPacket packet;
    char* RawBuffer;

public:
    // Constructors 
    PktDef();
    PktDef(char* raw);

    // Setters 
    void SetCmd(CmdType cmd);
    void SetBodyData(char* data, int size);
    void SetPktCount(int count);

    // Getters 
    CmdType GetCmd();
    bool GetAck();
    int GetLength();
    char* GetBodyData();
    int GetPktCount();

    // CRC 
    bool CheckCRC(char* buffer, int size);
    void CalcCRC();

    // Serialization 
    char* GenPacket();
};

#endif

