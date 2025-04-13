#include "../MileStone_2/MySocket.h"
#include "CppUnitTest.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace UnitTest2
{
	TEST_CLASS(UnitTest2)
	{
	public:
		
        TEST_METHOD(TestCase1_Constructor_DefaultSize)
        {
            MySocket sock(CLIENT, "127.0.0.1", 8080, TCP, 0);
            Assert::AreEqual(8080, sock.GetPort());
            Assert::AreEqual(std::string("127.0.0.1"), sock.GetIPAddr());
        }

        TEST_METHOD(TestCase2_SetIPAddr_Succeeds_WhenNotConnected)
        {
            MySocket sock(CLIENT, "127.0.0.1", 8080, TCP, 512);

            try {
                sock.ConnectTCP();  // Likely to fail (no server)
            }
            catch (...) {
                // Connection failed, so bTCPConnect is still false
            }

            sock.SetIPAddr("192.168.0.1");  // Should succeed
            Assert::AreEqual(std::string("192.168.0.1"), sock.GetIPAddr());
        }

        TEST_METHOD(TestCase3_SetPort_Succeeds_WhenNotConnected)
        {
            MySocket sock(CLIENT, "127.0.0.1", 8080, TCP, 512);

            try {
                sock.ConnectTCP();  // Will throw if no server
            }
            catch (...) {
                // No connection
            }

            sock.SetPort(9090);  // Should succeed
            Assert::AreEqual(9090, sock.GetPort());
        }


        TEST_METHOD(TestCase4_SocketType_Set_Get)
        {
            MySocket sock(CLIENT, "127.0.0.1", 9000, UDP, 512);
            Assert::AreEqual((int)CLIENT, (int)sock.GetType());
            sock.SetType(SERVER);
            Assert::AreEqual((int)SERVER, (int)sock.GetType());
        }

        TEST_METHOD(TestCase5_UDP_SendReceive_Loopback)
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