#include "PktDef.h"
#include <iostream>
#include <string>
#include <bitset>
#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment(lib, "Ws2_32.lib")  // Link with Winsock library

using namespace std;

// Helper function to print packet details to console
void PrintPkt(PktDef& pkt) {
    cout << "PktCount: " << pkt.GetPktCount() << endl;

    cout << "Command: ";
    switch (pkt.GetCmd()) {
    case PktDef::DRIVE: cout << "DRIVE"; break;
    case PktDef::SLEEP: cout << "SLEEP"; break;
    case PktDef::RESPONSE: cout << "RESPONSE"; break;
    }
    cout << endl;

    cout << "Ack: " << (pkt.GetAck() ? "Yes" : "No") << endl;
    cout << "Length: " << pkt.GetLength() << endl;

    // Print body bytes as integers
    char* body = pkt.GetBodyData();
    if (body) {
        cout << "Body Data: ";
        for (int i = 0; i < pkt.GetLength() - PktDef::HEADERSIZE; i++) {
            cout << (int)(unsigned char)body[i] << " ";
        }
        cout << endl;
    }
}

// Sends a packet via UDP, then waits for and returns the response
char* sendPkt(PktDef pkt) {
    // Generate raw packet bytes
    char* data = pkt.GenPacket();
    int pktSize = pkt.GetLength();

    // Initialize Winsock
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        cerr << "WSAStartup failed." << endl;
        return nullptr;
    }

    // Create UDP socket
    SOCKET sendSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sendSocket == INVALID_SOCKET) {
        cerr << "Socket creation failed." << endl;
        WSACleanup();
        return nullptr;
    }

    // Configure destination address (robot)
    sockaddr_in destAddr{};
    destAddr.sin_family = AF_INET;
    destAddr.sin_port = htons(5000); // Robot port
    inet_pton(AF_INET, "127.0.0.1", &destAddr.sin_addr); // Localhost for testing

    // Send the packet
    int sent = sendto(sendSocket, data, pktSize, 0, (sockaddr*)&destAddr, sizeof(destAddr));
    if (sent == SOCKET_ERROR) {
        cerr << "Send failed: " << WSAGetLastError() << endl;
        closesocket(sendSocket);
        WSACleanup();
        return nullptr;
    }

    cout << "Sent " << sent << " bytes to 127.0.0.1:5000" << endl;

    // Wait for response
    sockaddr_in fromAddr{};
    int fromAddrLen = sizeof(fromAddr);
    char* RxBuffer = new char[1024] {}; // Allocate buffer for incoming packet

    int received = recvfrom(sendSocket, RxBuffer, 1024, 0, (sockaddr*)&fromAddr, &fromAddrLen);
    if (received == SOCKET_ERROR) {
        cerr << "Receive failed: " << WSAGetLastError() << endl;
    }
    else {
        cout << "Received " << received << " bytes from robot." << endl;
    }

    // Clean up
    closesocket(sendSocket);
    WSACleanup();

    return RxBuffer; // Return raw buffer (caller is responsible for deletion)
}

int main() {
    int pktCounter = 0;

    // Main command loop
    while (true) {
        int direction = 0, duration = 0, speed = 0, option = 0;

        // Ask user for direction
        cout << "\n1. Forward\n2. Backward\n3. Right\n4. Left\n";
        cout << "Select direction: ";
        cin >> option;
        switch (option) {
        case 1: direction = PktDef::FORWARD; break;
        case 2: direction = PktDef::BACKWARD; break;
        case 3: direction = PktDef::RIGHT; break;
        case 4: direction = PktDef::LEFT; break;
        default: cout << "Invalid option.\n"; continue; // Restart loop on invalid input
        }

        // Ask user for duration and speed
        cout << "Enter duration (in seconds): ";
        cin >> duration;
        cout << "Enter speed (80–100): ";
        cin >> speed;

        // Validate speed
        if (speed < 80 || speed > 100) {
            cout << "The speed must be between 80 and 100.\n";
            continue;
        }

        // Construct drive command body: [direction, duration, speed]
        char body[3] = { (char)direction, (char)duration, (char)speed };

        // Create packet and set fields
        PktDef pkt;
        pkt.SetCmd(PktDef::DRIVE);
        pkt.SetPktCount(++pktCounter);
        pkt.SetBodyData(body, 3);

        // Send packet and wait for response
        char* responseRaw = sendPkt(pkt);
        if (responseRaw) {
            // Parse response and print details
            PktDef responsePkt(responseRaw);
            cout << " Received Packet " << endl;
            PrintPkt(responsePkt);
            delete[] responseRaw; // Clean up response buffer
        }
        else {
            cout << "No response." << endl;
        }
    }

    return 0;
}
