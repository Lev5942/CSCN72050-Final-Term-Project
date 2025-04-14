#include "MySocket.h"

// Constructor initializes socket settings and creates TCP/UDP socket
MySocket::MySocket(SocketType type, std::string ip, unsigned int port, ConnectionType connType, unsigned int size)
    : mySocket(type), IPAddr(ip), Port(port), connectionType(connType), bTCPConnect(false), MaxSize(size) {

    // Set default buffer size if invalid
    if (MaxSize <= 0) MaxSize = DEFAULT_SIZE;
    Buffer = new char[MaxSize]; // Allocate buffer memory

    // Initialize Winsock
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        throw std::runtime_error("WSAStartup failed");
    }

    // Zero out address struct and set address family and port
    memset(&SvrAddr, 0, sizeof(SvrAddr));
    SvrAddr.sin_family = AF_INET;
    SvrAddr.sin_port = htons(Port);
    inet_pton(AF_INET, IPAddr.c_str(), &SvrAddr.sin_addr); // Convert IP to binary form

    // Create the socket (TCP or UDP)
    if (connectionType == TCP) {
        ConnectionSocket = socket(AF_INET, SOCK_STREAM, 0);
        if (ConnectionSocket == INVALID_SOCKET) {
            throw std::runtime_error("TCP socket creation failed");
        }
    }
    else {
        ConnectionSocket = socket(AF_INET, SOCK_DGRAM, 0);
        if (ConnectionSocket == INVALID_SOCKET) {
            throw std::runtime_error("UDP socket creation failed");
        }
    }
}


// Destructor: clean up allocated memory and close the socket
MySocket::~MySocket() {
    delete[] Buffer;
    WSACleanup();
    closesocket(ConnectionSocket);
}

// General connection function: tries TCP and falls back to UDP if needed
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

// TCP-specific connection handling for both client and server roles
void MySocket::ConnectTCP() {
    if (mySocket == CLIENT) {
        if (connect(ConnectionSocket, (sockaddr*)&SvrAddr, sizeof(SvrAddr)) == SOCKET_ERROR) {
            throw std::runtime_error("TCP client connection failed");
        }
        bTCPConnect = true;
        std::cout << "TCP connection established.\n";
    }
    else if (mySocket == SERVER) {
        // Bind and listen for incoming connection
        if (bind(ConnectionSocket, (sockaddr*)&SvrAddr, sizeof(SvrAddr)) == SOCKET_ERROR) {
            throw std::runtime_error("TCP server bind failed");
        }
        listen(ConnectionSocket, SOMAXCONN);

        // Accept incoming connection
        sockaddr_in clientAddr;
        int clientSize = sizeof(clientAddr);
        ConnectionSocket = accept(ConnectionSocket, (sockaddr*)&clientAddr, &clientSize);
        if (ConnectionSocket == INVALID_SOCKET) {
            throw std::runtime_error("TCP server accept failed");
        }
        std::cout << "TCP server connection established.\n";
    }
}

// UDP-specific connection (only binds in server mode)
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

// TCP disconnection (close the socket)
void MySocket::DisconnectTCP() {
    if (bTCPConnect) {
        closesocket(ConnectionSocket);
        bTCPConnect = false;
    }
}

// Send data over TCP or UDP depending on mode
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

// Receive data over TCP or UDP into the provided output buffer
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

// Get the configured IP address
std::string MySocket::GetIPAddr() {
    return IPAddr;
}

// Set a new IP address (only if not connected)
void MySocket::SetIPAddr(std::string ip) {
    if (bTCPConnect) {
        std::cerr << "Cannot change IP address after connection established.\n";
        return;
    }
    IPAddr = ip;
    inet_pton(AF_INET, IPAddr.c_str(), &SvrAddr.sin_addr);
}

// Set a new port number (only if not connected)
void MySocket::SetPort(int port) {
    if (bTCPConnect) {
        std::cerr << "Cannot change port after connection established.\n";
        return;
    }
    Port = port;
    SvrAddr.sin_port = htons(Port);
}

// Return current port
int MySocket::GetPort() {
    return Port;
}

// Return current socket role (CLIENT or SERVER)
SocketType MySocket::GetType() {
    return mySocket;
}

// Set a new socket role (only if not connected)
void MySocket::SetType(SocketType type) {
    if (bTCPConnect) {
        std::cerr << "Cannot change socket type after connection established.\n";
        return;
    }
    mySocket = type;
}