#pragma once
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include <string>

#pragma comment(lib, "ws2_32.lib")

enum SocketType { CLIENT, SERVER };
enum ConnectionType { TCP, UDP };
const int DEFAULT_SIZE = 1024;

class MySocket {
private:
    char* Buffer;
    SOCKET WelcomeSocket;
    SOCKET ConnectionSocket;
    sockaddr_in SvrAddr;
    SocketType mySocket;
    std::string IPAddr;
    int Port;
    ConnectionType connectionType;
    bool bTCPConnect;
    int MaxSize;

public:
    MySocket(SocketType, std::string, unsigned int, ConnectionType, unsigned int);
    ~MySocket();

    void ConnectTCP();
    void DisconnectTCP();
    void SendData(const char*, int);
    int GetData(char*);

    std::string GetIPAddr();
    void SetIPAddr(std::string);
    void SetPort(int);
    int GetPort();
    SocketType GetType();
    void SetType(SocketType);
};
