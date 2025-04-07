#include "../MileStone_1/PktDef.h"
#include "CppUnitTest.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace UnitTest1
{
	TEST_CLASS(UnitTest1)
	{
	public:
		
        //passed
        TEST_METHOD(SetAndGetPktCount)
        {
            PktDef pkt;
            pkt.SetPktCount(42);
            Assert::AreEqual(42, pkt.GetPktCount());
        }

        //passed
        TEST_METHOD(SetAndGetCmd_Drive)
        {
            PktDef pkt;
            pkt.SetCmd(PktDef::DRIVE);
            Assert::AreEqual((int)PktDef::DRIVE, (int)pkt.GetCmd());
        }

        //passed
        TEST_METHOD(SetAndGetCmd_Sleep)
        {
            PktDef pkt;
            pkt.SetCmd(PktDef::SLEEP);
            Assert::AreEqual((int)PktDef::SLEEP, (int)pkt.GetCmd());
        }

        //passed
        TEST_METHOD(SetAndGetCmd_Response)
        {
            PktDef pkt;
            pkt.SetCmd(PktDef::RESPONSE);
            Assert::AreEqual((int)PktDef::RESPONSE, (int)pkt.GetCmd());
        }

        TEST_METHOD(SetAndGetBodyData)
        {
            PktDef pkt;
            char data[] = { PktDef::FORWARD, 5, 90 };
            pkt.SetBodyData(data, 3);
            char* body = pkt.GetBodyData();

            Assert::AreEqual<int>((int)PktDef::FORWARD, (int)body[0]);
            Assert::AreEqual<int>(5, (int)body[1]);
            Assert::AreEqual<int>(90, (int)body[2]);
        }

        //failed
        TEST_METHOD(SerializeDeserializePacket)
        {
            PktDef pkt;
            pkt.SetPktCount(1234);
            pkt.SetCmd(PktDef::DRIVE);
            char data[] = { PktDef::LEFT, 10, 100 };
            pkt.SetBodyData(data, 3);

            char* raw = pkt.GenPacket();
            int len = pkt.GetLength();

            PktDef pkt2(raw);
            char* body = pkt2.GetBodyData();
            
            //check this line
            Assert::IsNotNull(body);
            Assert::AreEqual(1234, pkt2.GetPktCount());
            Assert::AreEqual<int>((int)PktDef::DRIVE, (int)pkt2.GetCmd());
            Assert::AreEqual<int>((int)PktDef::LEFT, (int)body[0]);
            Assert::AreEqual<int>(10, (int)body[1]);
            Assert::AreEqual<int>(100, (int)body[2]);
        }


        //failed //probably something wrong with the GenPacket function recheck
        TEST_METHOD(CRCCheckValid)
        {
            PktDef pkt;
            pkt.SetPktCount(1);
            pkt.SetCmd(PktDef::DRIVE);
            char data[] = { PktDef::RIGHT, 8, 85 };
            pkt.SetBodyData(data, 3);

            //check this line
            char* raw = pkt.GenPacket();
            Assert::IsTrue(pkt.CheckCRC(raw, pkt.GetLength()));
        }

        //passed
        TEST_METHOD(ManualAckFlagDetection)
        {
            PktDef pkt;
            pkt.SetCmd(PktDef::RESPONSE);
            pkt.SetPktCount(55);
            pkt.SetBodyData(nullptr, 0);
            char* raw = pkt.GenPacket();

            raw[2] |= (1 << 4); // Set Ack bit manually

            PktDef pkt2(raw);
            Assert::IsTrue(pkt2.GetAck());
        }

        //failed
        TEST_METHOD(EmptyBodyPacket)
        {
            PktDef pkt;
            pkt.SetCmd(PktDef::SLEEP);
            pkt.SetPktCount(999);
            pkt.SetBodyData(nullptr, 0);

            //check this line 
            char* raw = pkt.GenPacket();
            PktDef pkt2(raw);

            Assert::AreEqual(PktDef::HEADERSIZE +1 , pkt2.GetLength());
            Assert::AreEqual(999, pkt2.GetPktCount());
            Assert::AreEqual((int)PktDef::SLEEP, (int)pkt2.GetCmd());
        }
	};
}
