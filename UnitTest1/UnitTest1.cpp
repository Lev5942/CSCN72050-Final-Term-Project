#include "../MileStone_1/PktDef.h"
#include "CppUnitTest.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace UnitTest1
{
	TEST_CLASS(UnitTest1)
	{
	public:

        // Test that SetPktCount correctly sets and retrieves the packet counter
        TEST_METHOD(TestCase1_SetAndGetPktCount)
        {
            PktDef pkt;
            pkt.SetPktCount(42);
            Assert::AreEqual(42, pkt.GetPktCount());
        }

        // Test setting and getting the DRIVE command
        TEST_METHOD(TestCase2_SetAndGetCmd_Drive)
        {
            PktDef pkt;
            pkt.SetCmd(PktDef::DRIVE);
            Assert::AreEqual((int)PktDef::DRIVE, (int)pkt.GetCmd());
        }

        // Test setting and getting the SLEEP command
        TEST_METHOD(TestCase3_SetAndGetCmd_Sleep)
        {
            PktDef pkt;
            pkt.SetCmd(PktDef::SLEEP);
            Assert::AreEqual((int)PktDef::SLEEP, (int)pkt.GetCmd());
        }

        // Test setting and getting the RESPONSE command
        TEST_METHOD(TestCase4_SetAndGetCmd_Response)
        {
            PktDef pkt;
            pkt.SetCmd(PktDef::RESPONSE);
            Assert::AreEqual((int)PktDef::RESPONSE, (int)pkt.GetCmd());
        }

        // Test that SetBodyData and GetBodyData work correctly for drive command
        TEST_METHOD(TestCase5_SetAndGetBodyData)
        {
            PktDef pkt;
            char data[] = { PktDef::FORWARD, 5, 90 }; // Direction, Duration, Speed
            pkt.SetBodyData(data, 3);
            char* body = pkt.GetBodyData();

            Assert::AreEqual<int>((int)PktDef::FORWARD, (int)body[0]);
            Assert::AreEqual<int>(5, (int)body[1]);
            Assert::AreEqual<int>(90, (int)body[2]);
        }

        // Test that CRC check passes on a properly built packet
        TEST_METHOD(TestCase6_CRCCheckValid)
        {
            PktDef pkt;
            pkt.SetPktCount(1);
            pkt.SetCmd(PktDef::DRIVE);
            char data[] = { PktDef::RIGHT, 8, 85 };
            pkt.SetBodyData(data, 3);

            char* raw = pkt.GenPacket();
            Assert::IsTrue(pkt.CheckCRC(raw, pkt.GetLength()));
        }

        // Manually set the Ack bit in the raw packet and ensure GetAck detects it
        TEST_METHOD(TestCase7_ManualAckFlagDetection)
        {
            PktDef pkt;
            pkt.SetCmd(PktDef::RESPONSE);
            pkt.SetPktCount(55);
            pkt.SetBodyData(nullptr, 0);
            char* raw = pkt.GenPacket();

            raw[2] |= (1 << 4); // Set bit 4 (Ack bit) manually

            PktDef pkt2(raw);
            Assert::IsTrue(pkt2.GetAck());
        }

        // Similar to above, verifies Ack bit can be set manually and retrieved
        TEST_METHOD(TestCase8_TestAckBitSetAndRetrieved)
        {
            PktDef pkt;
            pkt.SetPktCount(77);
            pkt.SetCmd(PktDef::DRIVE);
            char data[] = { 1, 2, 3 };
            pkt.SetBodyData(data, 3);

            // Manually set Ack flag in the raw buffer
            char* raw = pkt.GenPacket();
            raw[2] |= 0x10;  // Set bit 4 (Ack)

            PktDef pktIn(raw);
            Assert::IsTrue(pktIn.GetAck());
        }

        // Test that telemetry data can be set, packed, and parsed back correctly
        TEST_METHOD(TestCase9_TestTelemetrySetAndParse)
        {
            PktDef pkt;
            // Set telemetry data
            pkt.SetTelemetryData(555, 80, 4, 1, 3, 95);

            char* raw = pkt.GenPacket();    // Serialize
            PktDef pktIn(raw);              // Parse new instance from raw buffer

            // Confirm telemetry is present and all fields match
            Assert::IsTrue(pktIn.HasTelemetry());
            auto tele = pktIn.GetTelemetry();
            Assert::AreEqual(555, (int)tele.LastPktCounter);
            Assert::AreEqual(80, (int)tele.CurrentGrade);
            Assert::AreEqual(4, (int)tele.HitCount);
            Assert::AreEqual(1, (int)tele.LastCmd);
            Assert::AreEqual(3, (int)tele.LastCmdValue);
            Assert::AreEqual(95, (int)tele.LastCmdSpeed);
        }

        // Test that packet length is calculated correctly
        TEST_METHOD(TestCase10_TestLengthCalculation)
        {
            PktDef pkt;
            char data[] = { 10, 20 }; // 2-byte body
            pkt.SetBodyData(data, 2);

            // Check that total length is header + body
            Assert::AreEqual(PktDef::HEADERSIZE + 2, pkt.GetLength());
        }
	};
}


