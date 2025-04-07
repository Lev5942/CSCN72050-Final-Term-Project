#include "PktDef.h"
#include <iostream>
#include <string>
#include <bitset>
#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment(lib, "Ws2_32.lib")

using namespace std;

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
    char* body = pkt.
        GetBodyData();
    if (body) {
        cout << "Body Data: ";
        for (int i = 0; i < pkt.GetLength() - PktDef::HEADERSIZE; i++) {
            cout << (int)(unsigned char)body[i] << " ";
        }
        cout << endl;
    }
}

char* sendPkt(PktDef pkt) {
    char* data = pkt.GenPacket();
    int pktSize = pkt.GetLength();

    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        cerr << "WSAStartup failed." << endl;
        return nullptr;
    }

    SOCKET sendSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sendSocket == INVALID_SOCKET) {
        cerr << "Socket creation failed." << endl;
        WSACleanup();
        return nullptr;
    }

    sockaddr_in destAddr{};
    destAddr.sin_family = AF_INET;
    destAddr.sin_port = htons(5000);
    inet_pton(AF_INET, "127.0.0.1", &destAddr.sin_addr);

    int sent = sendto(sendSocket, data, pktSize, 0, (sockaddr*)&destAddr, sizeof(destAddr));
    if (sent == SOCKET_ERROR) {
        cerr << "Send failed: " << WSAGetLastError() << endl;
        closesocket(sendSocket);
        WSACleanup();
        return nullptr;
    }

    cout << "Sent " << sent << " bytes to 127.0.0.1:5000" << endl;

    sockaddr_in fromAddr{};
    int fromAddrLen = sizeof(fromAddr);
    char* RxBuffer = new char[1024] {};

    int received = recvfrom(sendSocket, RxBuffer, 1024, 0, (sockaddr*)&fromAddr, &fromAddrLen);
    if (received == SOCKET_ERROR) {
        cerr << "Receive failed: " << WSAGetLastError() << endl;
    }
    else {
        cout << "Received " << received << " bytes from robot." << endl;
    }

    closesocket(sendSocket);
    WSACleanup();

    return RxBuffer;
}

int main() {
    int pktCounter = 0;

    while (true) {
        int direction = 0, duration = 0, speed = 0, option = 0;

        cout << "\n1. Forward\n2. Backward\n3. Right\n4. Left\n";
        cout << "Select direction: ";
        cin >> option;
        switch (option) {
        case 1: direction = PktDef::FORWARD; break;
        case 2: direction = PktDef::BACKWARD; break;
        case 3: direction = PktDef::RIGHT; break;
        case 4: direction = PktDef::LEFT; break;
        default: cout << "Invalid option.\n"; continue;
        }

        cout << "Enter duration (in seconds): ";
        cin >> duration;
        cout << "Enter speed (80–100): ";
        cin >> speed;

        if (speed < 80 || speed > 100) {
            cout << "The speed must be between 80 and 100.\n";
            continue;
        }

        char body[3] = { (char)direction, (char)duration, (char)speed };

        PktDef pkt;
        pkt.SetCmd(PktDef::DRIVE);
        pkt.SetPktCount(++pktCounter);
        pkt.SetBodyData(body, 3);

        char* responseRaw = sendPkt(pkt);
        if (responseRaw) {
            PktDef responsePkt(responseRaw);
            cout << " Received Packet " << endl;
            PrintPkt(responsePkt);
            delete[] responseRaw;
        }
        else {
            cout << "No response." << endl;
        }
    }

    return 0;
}


