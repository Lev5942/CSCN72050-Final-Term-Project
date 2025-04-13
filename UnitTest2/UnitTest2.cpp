#include "../MileStone_2/MySocket.h"
#include "CppUnitTest.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace UnitTest2
{
	TEST_CLASS(UnitTest2)
	{
	public:
		
        TEST_METHOD(Constructor_DefaultSize)
        {
            MySocket sock(CLIENT, "127.0.0.1", 8080, TCP, 0);
            Assert::AreEqual(8080, sock.GetPort());
            Assert::AreEqual(std::string("127.0.0.1"), sock.GetIPAddr());
        }

        TEST_METHOD(SetIPAddr_Fails_WhenConnected)
        {
            MySocket sock(CLIENT, "127.0.0.1", 8080, TCP, 512);
            try {
                sock.ConnectTCP();  // Likely to fail (no server), but sets bTCPConnect
            }
            catch (...) {}

            sock.SetIPAddr("192.168.0.1");  // Should not update
            Assert::AreEqual(std::string("127.0.0.1"), sock.GetIPAddr());
        }

        TEST_METHOD(SetPort_Fails_WhenConnected)
        {
            MySocket sock(CLIENT, "127.0.0.1", 8080, TCP, 512);
            try {
                sock.ConnectTCP();  // Might throw
            }
            catch (...) {}

            sock.SetPort(9090);
            Assert::AreEqual(8080, sock.GetPort());
        }

        TEST_METHOD(SocketType_Set_Get)
        {
            MySocket sock(CLIENT, "127.0.0.1", 9000, UDP, 512);
            Assert::AreEqual((int)CLIENT, (int)sock.GetType());
            sock.SetType(SERVER);
            Assert::AreEqual((int)SERVER, (int)sock.GetType());
        }

        TEST_METHOD(UDP_SendReceive_Loopback)
        {
            // Setup server
            MySocket server(SERVER, "127.0.0.1", 8050, UDP, 1024);

            // Setup client
            MySocket client(CLIENT, "127.0.0.1", 8050, UDP, 1024);

            const char* msg = "Hello UDP";
            client.SendData(msg, strlen(msg));

            char recvBuf[1024] = {};
            int bytes = server.GetData(recvBuf);

            Assert::AreEqual(strlen(msg), (size_t)bytes);
            Assert::AreEqual(std::string(msg), std::string(recvBuf, bytes));
        }
    };
}