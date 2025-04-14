#pragma once
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include <string>

#pragma comment(lib, "ws2_32.lib")

using namespace std;

enum SocketType { CLIENT, SERVER };
enum ConnectionType { TCP, UDP };
const int DEFAULT_SIZE = 1024;

class MySocket {
private:
    char* Buffer;
    SOCKET WelcomeSocket;
    SOCKET ConnectionSocket;
    struct sockaddr_in SvrAddr;
    SocketType mySocket;
    std::string IPAddr;
    int Port;
    ConnectionType connectionType;
    bool bTCPConnect;
    int MaxSize;
    WSADATA wsaData;

public:
    MySocket();
    MySocket(SocketType type, std::string ip, unsigned int port, ConnectionType connType, unsigned int size);
    ~MySocket();

    void ConnectTCP();
    void DisconnectTCP();
    void SendData(const char* data, int size);
    int GetData(char* outputBuffer);

    std::string GetIPAddr();
    void SetIPAddr(std::string ip);
    void SetPort(int port);
    int GetPort();
    SocketType GetType();
    void SetType(SocketType type);
    void Connect(); // This function will handle the fallback between TCP and UDP.
    void ConnectUDP(); // This function will establish a UDP connection.
};