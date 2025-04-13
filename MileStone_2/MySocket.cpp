#include "MySocket.h"

MySocket::MySocket(SocketType type, std::string ip, unsigned int port, ConnectionType connType, unsigned int size)
    : mySocket(type), IPAddr(ip), Port(port), connectionType(connType), bTCPConnect(false), MaxSize(size) {

    if (MaxSize <= 0) MaxSize = DEFAULT_SIZE;
    Buffer = new char[MaxSize];

    // Initialize Winsock
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
        throw std::runtime_error("WSAStartup failed");

    memset(&SvrAddr, 0, sizeof(SvrAddr));
    SvrAddr.sin_family = AF_INET;
    SvrAddr.sin_port = htons(Port);
    inet_pton(AF_INET, IPAddr.c_str(), &SvrAddr.sin_addr);

    if (connectionType == TCP) {
        if (mySocket == SERVER) {
            WelcomeSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
            if (WelcomeSocket == INVALID_SOCKET) throw std::runtime_error("TCP Server socket creation failed");

            if (bind(WelcomeSocket, (sockaddr*)&SvrAddr, sizeof(SvrAddr)) == SOCKET_ERROR)
                throw std::runtime_error("Bind failed");

            listen(WelcomeSocket, SOMAXCONN);
        }
        else {
            ConnectionSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
            if (ConnectionSocket == INVALID_SOCKET)
                throw std::runtime_error("TCP Client socket creation failed");
        }
    }
    else {
        ConnectionSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
        if (ConnectionSocket == INVALID_SOCKET)
            throw std::runtime_error("UDP socket creation failed");

        if (mySocket == SERVER) {
            if (bind(ConnectionSocket, (sockaddr*)&SvrAddr, sizeof(SvrAddr)) == SOCKET_ERROR)
                throw std::runtime_error("UDP Server bind failed");
        }
    }
}

MySocket::~MySocket() {
    delete[] Buffer;
    if (connectionType == TCP && mySocket == SERVER)
        closesocket(WelcomeSocket);
    closesocket(ConnectionSocket);
    WSACleanup();
}

void MySocket::ConnectTCP() {
    if (connectionType != TCP) throw std::logic_error("ConnectTCP() not allowed for UDP");

    if (mySocket == CLIENT) {
        if (connect(ConnectionSocket, (sockaddr*)&SvrAddr, sizeof(SvrAddr)) == SOCKET_ERROR)
            throw std::runtime_error("TCP Client connect failed");
        bTCPConnect = true;
    }
    else if (mySocket == SERVER) {
        int addrlen = sizeof(SvrAddr);
        ConnectionSocket = accept(WelcomeSocket, (sockaddr*)&SvrAddr, &addrlen);
        if (ConnectionSocket == INVALID_SOCKET)
            throw std::runtime_error("TCP Server accept failed");
        bTCPConnect = true;
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
        send(ConnectionSocket, data, size, 0);
    }
    else {
        sendto(ConnectionSocket, data, size, 0, (sockaddr*)&SvrAddr, sizeof(SvrAddr));
    }
}

int MySocket::GetData(char* outputBuffer) {
    int bytesRead = 0;
    int addrLen = sizeof(SvrAddr);

    if (connectionType == TCP) {
        bytesRead = recv(ConnectionSocket, Buffer, MaxSize, 0);
    }
    else {
        bytesRead = recvfrom(ConnectionSocket, Buffer, MaxSize, 0, (sockaddr*)&SvrAddr, &addrLen);
    }

    if (bytesRead > 0)
        memcpy(outputBuffer, Buffer, bytesRead);

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
    if (bTCPConnect || (connectionType == TCP && mySocket == SERVER && WelcomeSocket != INVALID_SOCKET)) {
        std::cerr << "Cannot change socket type while active.\n";
        return;
    }
    mySocket = type;
}
