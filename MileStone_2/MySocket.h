#pragma once

// Winsock headers for socket programming on Windows
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include <string>

// Link against Winsock library
#pragma comment(lib, "ws2_32.lib")

// Enum to define whether the socket is for client or server use
enum SocketType { CLIENT, SERVER };

// Enum to define the connection type: TCP or UDP
enum ConnectionType { TCP, UDP };

// Default buffer size for data transmission
const int DEFAULT_SIZE = 1024;

class MySocket {
private:
    char* Buffer;               // Dynamic buffer to store received data
    SOCKET WelcomeSocket;       // (Unused in current implementation, can be used for TCP welcome socket)
    SOCKET ConnectionSocket;    // Socket used for sending/receiving data
    sockaddr_in SvrAddr;        // Server address structure
    SocketType mySocket;        // Indicates whether this instance is a client or server
    std::string IPAddr;         // Target IP address as a string
    int Port;                   // Port number for connection
    ConnectionType connectionType; // TCP or UDP
    bool bTCPConnect;           // True if TCP connection is active
    int MaxSize;                // Max buffer size for data
    WSADATA wsaData;            // Winsock startup data

public:
    // Constructor: Initializes socket with type, IP, port, connection type, and buffer size
    MySocket(SocketType type, std::string ip, unsigned int port, ConnectionType connType, unsigned int size);

    // Destructor: Cleans up resources
    ~MySocket();

    // Establish TCP connection (client or server based on mySocket)
    void ConnectTCP();

    // Disconnect TCP connection
    void DisconnectTCP();

    // Send data through socket
    void SendData(const char* data, int size);

    // Receive data into provided buffer
    int GetData(char* outputBuffer);

    // Get/Set IP address
    std::string GetIPAddr();
    void SetIPAddr(std::string ip);

    // Get/Set port
    void SetPort(int port);
    int GetPort();

    // Get/Set socket type (client/server)
    SocketType GetType();
    void SetType(SocketType type);

    // Automatically handles connection setup based on connectionType
    void Connect();

    // Establish UDP bind (for server) or prep socket for sending (for client)
    void ConnectUDP();
};