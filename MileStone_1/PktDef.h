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
    static const int HEADERSIZE = 4; 

    // Enums 
    enum CmdType { DRIVE, SLEEP, RESPONSE };

    // Structs 
    struct Header {
        unsigned short int PktCount;
        unsigned char Status : 1;
        unsigned char Drive : 1;
        unsigned char Sleep : 1;
        unsigned char Ack : 1;
        unsigned char Padding : 4;
        unsigned short int Length : 8;
    }HEADER;

    struct Drivebody {
        unsigned char Direction;
        unsigned char Duration;
        unsigned char Speed;
    }DRIVEBODY;

    struct Telemetry {
        unsigned short int LastPktCounter;
        unsigned short int CurrentGrade;
        unsigned short int HitCount;
        unsigned char LastCmd;
        unsigned char LastCmdValue;
        unsigned char LastCmdSpeed;
    }TELEMETRY;

private:
    struct CmdPacket {
        Header header;
        char* Data;
        unsigned char CRC;
    };

    CmdPacket packet;
    char* RawBuffer;

    Telemetry telemetryData;
    bool hasTelemetry = false;

public:
    // Constructors 
    PktDef();
    PktDef(char* raw);

    //Destructor
    ~PktDef();

    // Setters 
    void SetCmd(CmdType cmd);
    void SetBodyData(char* data, int size);
    void SetPktCount(int count);
    void SetTelemetryData(unsigned short int lastPkt, unsigned short int grade, unsigned short int hits,
        unsigned char cmd, unsigned char val, unsigned char spd);

    // Getters 
    CmdType GetCmd();
    bool GetAck();
    int GetLength();
    char* GetBodyData();
    int GetPktCount();
    bool HasTelemetry();
    Telemetry GetTelemetry();

    // CRC 
    bool CheckCRC(char* buffer, int size);
    void CalcCRC();


    // Serialization 
    char* GenPacket();
};

#endif
