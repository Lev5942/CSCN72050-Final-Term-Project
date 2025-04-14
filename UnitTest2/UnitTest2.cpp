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
                sock.ConnectTCP();
            }
            catch (...) {}

            sock.SetIPAddr("192.168.0.1");
            Assert::AreEqual(std::string("192.168.0.1"), sock.GetIPAddr());
        }

        TEST_METHOD(TestCase3_SetPort_Succeeds_WhenNotConnected)
        {
            MySocket sock(CLIENT, "127.0.0.1", 8080, TCP, 512);

            try {
                sock.ConnectTCP();
            }
            catch (...) {}

            sock.SetPort(9090);
            Assert::AreEqual(9090, sock.GetPort());
        }

        TEST_METHOD(TestCase4_SocketType_Set_Get)
        {
            MySocket sock(CLIENT, "127.0.0.1", 9000, UDP, 512);
            Assert::AreEqual((int)CLIENT, (int)sock.GetType());
            sock.SetType(SERVER);
            Assert::AreEqual((int)SERVER, (int)sock.GetType());
        }

        TEST_METHOD(TestCase5_Constructor_ValidParams)
        {
            MySocket socket(CLIENT, "127.0.0.1", 8080, TCP, 1024);

            Assert::AreEqual(std::string("127.0.0.1"), socket.GetIPAddr());
            Assert::AreEqual(8080, socket.GetPort());
            Assert::AreEqual(CLIENT, socket.GetType());
        }

        TEST_METHOD(TestCase6_ConnectTCP_Client)
        {
            MySocket socket(CLIENT, "127.0.0.1", 8080, TCP, 1024);
            try {
                socket.Connect();
            }
            catch (const std::exception& e) {
                Logger::WriteMessage(e.what());
                Assert::Fail(L"Exception thrown during TCP Client Connect.");
            }
            socket.DisconnectTCP();
        }

        TEST_METHOD(TestCase7_ConnectUDP_Client)
        {
            MySocket socket(CLIENT, "127.0.0.1", 8080, UDP, 1024);
            try {
                socket.Connect();
            }
            catch (const std::exception& e) {
                Logger::WriteMessage(e.what());
                Assert::Fail(L"Exception thrown during UDP Client Connect.");
            }
        }

        TEST_METHOD(TestCase8_ConnectTCP_Server)
        {
            MySocket socket(SERVER, "127.0.0.1", 8080, TCP, 1024);
            try {
                socket.Connect();
            }
            catch (const std::exception& e) {
                Logger::WriteMessage(e.what());
                Assert::Fail(L"Exception thrown during TCP Server Connect.");
            }
            socket.DisconnectTCP();
        }

        TEST_METHOD(TestCase9_ConnectUDP_Server)
        {
            MySocket socket(SERVER, "127.0.0.1", 8080, UDP, 1024);
            try {
                socket.Connect();
            }
            catch (const std::exception& e) {
                Logger::WriteMessage(e.what());
                Assert::Fail(L"Exception thrown during UDP Server Connect.");
            }
        }

        TEST_METHOD(TestCase10_SendData_TCP)
        {
            MySocket socket(CLIENT, "127.0.0.1", 8080, TCP, 1024);
            try {
                socket.Connect();
                const char* testData = "Test Data";
                socket.SendData(testData, static_cast<int>(strlen(testData) + 1));
            }
            catch (const std::exception& e) {
                Logger::WriteMessage(e.what());
                Assert::Fail(L"Exception thrown during SendData (TCP).");
            }
            socket.DisconnectTCP();
        }

        TEST_METHOD(TestCase11_SendData_UDP)
        {
            MySocket socket(CLIENT, "127.0.0.1", 8080, UDP, 1024);
            try {
                socket.Connect();
                const char* testData = "Test Data";
                socket.SendData(testData, static_cast<int>(strlen(testData) + 1));
            }
            catch (const std::exception& e) {
                Logger::WriteMessage(e.what());
                Assert::Fail(L"Exception thrown during SendData (UDP).");
            }
        }

        TEST_METHOD(TestCase12_GetData_TCP)
        {
            MySocket socket(SERVER, "127.0.0.1", 8080, TCP, 1024);
            socket.Connect();
            char buffer[1024];
            int bytesRead = socket.GetData(buffer);
            Assert::IsTrue(bytesRead > 0);
            socket.DisconnectTCP();
        }

        TEST_METHOD(TestCase13_GetData_UDP)
        {
            MySocket socket(SERVER, "127.0.0.1", 8080, UDP, 1024);
            socket.Connect();
            char buffer[1024];
            int bytesRead = socket.GetData(buffer);
            Assert::IsTrue(bytesRead > 0);
        }

        TEST_METHOD(TestCase14_SetGetIPAddr)
        {
            MySocket socket(CLIENT, "127.0.0.1", 8080, TCP, 1024);
            Assert::AreEqual(std::string("127.0.0.1"), socket.GetIPAddr());

            socket.SetIPAddr("192.168.1.1");
            Assert::AreEqual(std::string("192.168.1.1"), socket.GetIPAddr());
        }

        TEST_METHOD(TestCase15_SetGetPort)
        {
            MySocket socket(CLIENT, "127.0.0.1", 8080, TCP, 1024);
            Assert::AreEqual(8080, socket.GetPort());

            socket.SetPort(9090);
            Assert::AreEqual(9090, socket.GetPort());
        }

        TEST_METHOD(TestCase16_SetType)
        {
            MySocket socket(CLIENT, "127.0.0.1", 8080, TCP, 1024);
            Assert::AreEqual(CLIENT, socket.GetType());

            socket.SetType(SERVER);
            Assert::AreEqual(SERVER, socket.GetType());
        }

        TEST_METHOD(TestCase17_DisconnectTCP)
        {
            MySocket socket(CLIENT, "127.0.0.1", 8080, TCP, 1024);
            socket.Connect();
            socket.DisconnectTCP();
            // Assuming disconnect does not change type, this might be an error. Adjust logic if needed:
            Assert::AreEqual(CLIENT, socket.GetType());
        }

        TEST_METHOD(TestCase18_InvalidPort)
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
