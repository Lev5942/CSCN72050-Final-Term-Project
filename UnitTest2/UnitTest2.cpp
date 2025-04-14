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

        TEST_METHOD(TestCase6_Constructor_ValidParams)
        {
            MySocket socket(CLIENT, "127.0.0.1", 8080, TCP, 1024);

            Assert::AreEqual(socket.GetIPAddr(), std::string("127.0.0.1"));
            Assert::AreEqual(socket.GetPort(), 8080);
            Assert::AreEqual(socket.GetType(), CLIENT);
        }

        TEST_METHOD(TestCase7_ConnectTCP_Client)
        {
            MySocket socket(CLIENT, "127.0.0.1", 8080, TCP, 1024);
            try {
                socket.Connect();
            }
            catch (const std::exception& e) {
                Assert::Fail(L"Exception thrown: " + std::wstring(e.what(), e.what() + strlen(e.what())));
            }
            socket.DisconnectTCP();
        }

        TEST_METHOD(TestCase8_ConnectUDP_Client)
        {
            MySocket socket(CLIENT, "127.0.0.1", 8080, UDP, 1024);
            try {
                socket.Connect();
            }
            catch (const std::exception& e) {
                Assert::Fail(L"Exception thrown: " + std::wstring(e.what(), e.what() + strlen(e.what())));
            }
        }

        TEST_METHOD(TestCase9_ConnectTCP_Server)
        {
            MySocket socket(SERVER, "127.0.0.1", 8080, TCP, 1024);
            try {
                socket.Connect();
            }
            catch (const std::exception& e) {
                Assert::Fail(L"Exception thrown: " + std::wstring(e.what(), e.what() + strlen(e.what())));
            }
            socket.DisconnectTCP();
        }

        TEST_METHOD(TestCase10_ConnectUDP_Server)
        {
            MySocket socket(SERVER, "127.0.0.1", 8080, UDP, 1024);
            try {
                socket.Connect();
            }
            catch (const std::exception& e) {
                Assert::Fail(L"Exception thrown: " + std::wstring(e.what(), e.what() + strlen(e.what())));
            }
        }

        TEST_METHOD(TestCase11_SendData_TCP)
        {
            MySocket socket(CLIENT, "127.0.0.1", 8080, TCP, 1024);
            socket.Connect();
            const char* testData = "Test Data";
            try {
                socket.SendData(testData, strlen(testData) + 1);
            }
            catch (const std::exception& e) {
                Assert::Fail(L"Exception thrown: " + std::wstring(e.what(), e.what() + strlen(e.what())));
            }
            socket.DisconnectTCP();
        }

        TEST_METHOD(TestCase12_SendData_UDP)
        {
            MySocket socket(CLIENT, "127.0.0.1", 8080, UDP, 1024);
            socket.Connect();
            const char* testData = "Test Data";
            try {
                socket.SendData(testData, strlen(testData) + 1);
            }
            catch (const std::exception& e) {
                Assert::Fail(L"Exception thrown: " + std::wstring(e.what(), e.what() + strlen(e.what())));
            }
        }

        TEST_METHOD(TestCase13_GetData_TCP)
        {
            MySocket socket(SERVER, "127.0.0.1", 8080, TCP, 1024);
            socket.Connect();
            char buffer[1024];
            int bytesRead = socket.GetData(buffer);
            Assert::IsTrue(bytesRead > 0);
            socket.DisconnectTCP();
        }

        TEST_METHOD(TestCase14_GetData_UDP)
        {
            MySocket socket(SERVER, "127.0.0.1", 8080, UDP, 1024);
            socket.Connect();
            char buffer[1024];
            int bytesRead = socket.GetData(buffer);
            Assert::IsTrue(bytesRead > 0);
        }

        TEST_METHOD(TestCase15_SetGetIPAddr)
        {
            MySocket socket(CLIENT, "127.0.0.1", 8080, TCP, 1024);
            Assert::AreEqual(socket.GetIPAddr(), std::string("127.0.0.1"));

            socket.SetIPAddr("192.168.1.1");
            Assert::AreEqual(socket.GetIPAddr(), std::string("192.168.1.1"));
        }

        TEST_METHOD(TestCase16_SetGetPort)
        {
            MySocket socket(CLIENT, "127.0.0.1", 8080, TCP, 1024);
            Assert::AreEqual(socket.GetPort(), 8080);

            socket.SetPort(9090);
            Assert::AreEqual(socket.GetPort(), 9090);
        }

        TEST_METHOD(TestCase17_SetType)
        {
            MySocket socket(CLIENT, "127.0.0.1", 8080, TCP, 1024);
            Assert::AreEqual(socket.GetType(), CLIENT);

            socket.SetType(SERVER);
            Assert::AreEqual(socket.GetType(), SERVER);
        }

        TEST_METHOD(TestCase18_DisconnectTCP)
        {
            MySocket socket(CLIENT, "127.0.0.1", 8080, TCP, 1024);
            socket.Connect();
            socket.DisconnectTCP();
            Assert::AreEqual(socket.GetType(), SERVER);
        }

        TEST_METHOD(TestCase19_InvalidPort)
        {
            MySocket socket(CLIENT, "127.0.0.1", 8080, TCP, 1024);
            try {
                socket.SetPort(-1);
                Assert::Fail(L"Expected exception for invalid port");
            }
            catch (const std::invalid_argument&) {
                // Expected exception.
            }
        }
    };
}
