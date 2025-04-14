#include "MySocket.h"

MySocket::MySocket() : bTCPConnect(false), MaxSize(DEFAULT_SIZE)
{
}

MySocket::MySocket(SocketType type, std::string ip, unsigned int port, ConnectionType connType, unsigned int bufferSize)
    : mySocket(type), IPAddr(ip), Port(port), connectionType(connType), bTCPConnect(false), MaxSize(DEFAULT_SIZE)
{
    // Step 1: Setup buffer size
    MaxSize = (bufferSize > 0) ? bufferSize : DEFAULT_SIZE;
    Buffer = new char[MaxSize];

    // Step 2: Initialize Winsock (Windows only)
    std::cout << "Initializing Winsock...\n";
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "WSAStartup failed\n";
        return;
    }

    // Step 3: Clear server address struct
    memset(&SvrAddr, 0, sizeof(SvrAddr));
    SvrAddr.sin_family = AF_INET;
    SvrAddr.sin_port = htons(Port);

    if (connectionType == UDP) {
        std::cout << "Setting up UDP socket...\n";
        ConnectionSocket = socket(AF_INET, SOCK_DGRAM, 0);
        if (ConnectionSocket == INVALID_SOCKET) {
            std::cerr << "ERROR: Failed to create UDP socket\n";
            return;
        }

        if (mySocket == CLIENT) {
            std::cout << "Setting up CLIENT (UDP)\n";
            inet_pton(AF_INET, ip.c_str(), &SvrAddr.sin_addr);
            std::cout << "CLIENT UDP setup successfully\n";
        }
        else if (mySocket == SERVER) {
            std::cout << "Setting up SERVER (UDP)\n";
            SvrAddr.sin_addr.s_addr = INADDR_ANY;
            if (bind(ConnectionSocket, (sockaddr*)&SvrAddr, sizeof(SvrAddr)) == SOCKET_ERROR) {
                std::cerr << "ERROR: Failed to bind UDP socket\n";
                closesocket(ConnectionSocket);
                return;
            }
            std::cout << "SERVER UDP setup successfully\n";
        }
    }
    else if (connectionType == TCP) {
        std::cout << "Setting up TCP socket...\n";

        if (mySocket == CLIENT) {
            std::cout << "Setting up CLIENT (TCP)\n";
            ConnectionSocket = socket(AF_INET, SOCK_STREAM, 0);
            if (ConnectionSocket == INVALID_SOCKET) {
                std::cerr << "ERROR: Failed to create TCP socket (CLIENT)\n";
                return;
            }

            inet_pton(AF_INET, ip.c_str(), &SvrAddr.sin_addr);
            std::cout << "CLIENT TCP set up successfully\n";
        }
        else if (mySocket == SERVER) {
            std::cout << "Setting up SERVER (TCP)\n";
            WelcomeSocket = socket(AF_INET, SOCK_STREAM, 0);
            if (WelcomeSocket == INVALID_SOCKET) {
                std::cerr << "ERROR: Failed to create TCP socket (SERVER)\n";
                return;
            }

            SvrAddr.sin_addr.s_addr = INADDR_ANY;
            std::cout << "SERVER TCP setup prepared (bind/listen/accept to be handled separately)\n";
        }
    }
}


MySocket::~MySocket() {
    delete[] Buffer;
    WSACleanup();
    closesocket(ConnectionSocket);
}

void MySocket::Connect() {
    if (connectionType == TCP) {
        try {
            ConnectTCP(); // Try TCP connection
        }
        catch (const std::exception& e) {
            std::cerr << "TCP connection failed: " << e.what() << "\n";
            std::cout << "Falling back to UDP...\n";
            ConnectUDP(); // Fall back to UDP if TCP fails
        }
    }
    else {
        ConnectUDP(); // If the socket is set to UDP, try connecting with UDP
    }
}

void MySocket::ConnectTCP() {
    if (mySocket == CLIENT) {
        if (connect(ConnectionSocket, (sockaddr*)&SvrAddr, sizeof(SvrAddr)) == SOCKET_ERROR) {
            throw std::runtime_error("TCP client connection failed");
        }
        bTCPConnect = true;
        std::cout << "TCP connection established.\n";
    }
    else if (mySocket == SERVER) {
        if (bind(ConnectionSocket, (sockaddr*)&SvrAddr, sizeof(SvrAddr)) == SOCKET_ERROR) {
            throw std::runtime_error("TCP server bind failed");
        }
        listen(ConnectionSocket, SOMAXCONN);
        sockaddr_in clientAddr;
        int clientSize = sizeof(clientAddr);
        ConnectionSocket = accept(ConnectionSocket, (sockaddr*)&clientAddr, &clientSize);
        if (ConnectionSocket == INVALID_SOCKET) {
            throw std::runtime_error("TCP server accept failed");
        }
        std::cout << "TCP server connection established.\n";
    }
}

void MySocket::ConnectUDP() {
    if (mySocket == SERVER) {
        if (bind(ConnectionSocket, (sockaddr*)&SvrAddr, sizeof(SvrAddr)) == SOCKET_ERROR) {
            throw std::runtime_error("UDP server bind failed");
        }
        std::cout << "UDP server is ready to receive data.\n";
    }
    else {
        std::cout << "UDP client is ready to send data.\n";
    }
}

void MySocket::DisconnectTCP() {
    if (bTCPConnect) {
        closesocket(ConnectionSocket);
        bTCPConnect = false;
    }
}

void MySocket::SendData(const char* data, int size) {
    if (connectionType == TCP) {
        if (send(ConnectionSocket, data, size, 0) == SOCKET_ERROR) {
            throw std::runtime_error("SendData failed for TCP");
        }
    }
    else {
        if (sendto(ConnectionSocket, data, size, 0, (sockaddr*)&SvrAddr, sizeof(SvrAddr)) == SOCKET_ERROR) {
            throw std::runtime_error("SendData failed for UDP");
        }
    }
}

int MySocket::GetData(char* outputBuffer) {
    int bytesRead = 0;
    sockaddr_in clientAddr;
    int addrLen = sizeof(clientAddr);

    if (connectionType == TCP) {
        bytesRead = recv(ConnectionSocket, Buffer, MaxSize, 0);
    }
    else {
        bytesRead = recvfrom(ConnectionSocket, Buffer, MaxSize, 0, (sockaddr*)&clientAddr, &addrLen);
    }

    if (bytesRead > 0) {
        memcpy(outputBuffer, Buffer, bytesRead);
    }

    return bytesRead;
}

std::string MySocket::GetIPAddr() {
    return IPAddr;
}

void MySocket::SetIPAddr(std::string ip) {
    if (bTCPConnect) {
        std::cerr << "Cannot change IP address after connection established.\n";
        return;
    }
    IPAddr = ip;
    inet_pton(AF_INET, IPAddr.c_str(), &SvrAddr.sin_addr);
}

void MySocket::SetPort(int port) {
    if (bTCPConnect) {
        std::cerr << "Cannot change port after connection established.\n";
        return;
    }
    Port = port;
    SvrAddr.sin_port = htons(Port);
}

int MySocket::GetPort() {
    return Port;
}

SocketType MySocket::GetType() {
    return mySocket;
}

void MySocket::SetType(SocketType type) {
    if (bTCPConnect) {
        std::cerr << "Cannot change socket type after connection established.\n";
        return;
    }
    mySocket = type;
}