#include "stdafx.h"
#include "Global.h"
#include "SRRdtReceiver.h"

int Position(int base, int WindowLen, int max_seqnum, int seq) {
	//当接受窗口连续时（不经过max_seqnum-1到0的区间）
	if (base < (base + WindowLen) % max_seqnum) 
	{
		if (seq >= base && seq < base + WindowLen)
			return 1;
		else 
			return 0;
	}
	//当接受窗口不连续时（中间经过max_seqnum-1到0的区间，一次循环往复）
	else 
	{
		if (seq >= base || seq < (base + WindowLen) % max_seqnum)
			return 1;
		else
			return 0;
	}
}

SRRdtReceiver::SRRdtReceiver() :base(0), buffernum(0), WindowLen(4), max_seqnum(8)
{
	Packet ackPkt;
	ackPkt.acknum = -1; //忽略该字段
	ackPkt.checksum = 0;
	ackPkt.seqnum = -1;	//忽略该字段
	for (int i = 0; i < Configuration::PAYLOAD_SIZE; i++) 
	{
		ackPkt.payload[i] = '.';
	}
	for (int i = 0; i < WindowLen; i++)
	{
		Pkt_vec.push_back(make_pair(ackPkt, false));   //缓冲序列初始化windowlen大小
	}
}


SRRdtReceiver::~SRRdtReceiver()
{
}

void SRRdtReceiver::receive(const Packet& packet) {
	//检查校验和是否正确
	int checkSum = pUtils->calculateCheckSum(packet);

	//如果校验和正确
	if (checkSum == packet.checksum)
	{
		//1、当收到的数据包在接受窗口范围之内
		if (Position(base, WindowLen, max_seqnum, packet.seqnum) == 1)
		{
			cout << endl;
			pUtils->printPacket("[SUCCEED]:RECEIVE AN EXPECTED PACKET", packet);
			cout << endl;
			int index = (packet.seqnum - base + max_seqnum) % max_seqnum;
			if (Pkt_vec[index].second == false)
			{
				Pkt_vec[index] = make_pair(packet, true);  //缓存数据包，设置已接收标志
				buffernum++;
			}
			//构造确认ackPkt
			Packet ackPkt;
			ackPkt.acknum = packet.seqnum;           //确认该数据
			ackPkt.seqnum = -1;
			for (int i = 0; i < Configuration::PAYLOAD_SIZE; i++) 
			{
				ackPkt.payload[i] = '.';
			}
			ackPkt.checksum = pUtils->calculateCheckSum(ackPkt);
			pUtils->printPacket("[SUCCEED]:RETURN ACK TO SENDER", ackPkt);
			pns->sendToNetworkLayer(SENDER, ackPkt);
			
			cout << endl << "------BEFORE MOVING \"RECEIVER\" WINDOW------(A: Empty buffer; B: Mis_order and buffered packet; C:Unusable )" << endl;
			
			cout << "***RECEIVER WINDOW: [ ";
			//numberA-C分别是A类、B类和C类的报文个数
			for (int i = 0; i < WindowLen; i++)
			{
				if (Pkt_vec[i].second == false)
					cout << (base + i) % max_seqnum << "-A" << " ";     //输出A类和B类报文(其中细分是否buffered or not）
				else if (Pkt_vec[i].second == true)
					cout << (base + i) % max_seqnum << "-B" << " ";
			}
			cout << "]";
			for (int i = 0; i < max_seqnum - WindowLen; i++)            //输出C类报文
				cout << (base + WindowLen + i) % max_seqnum << "-C" << " ";
			cout << endl << endl;

			//只有接收的seqnum == base接收方才会移动窗口，否则只做true标记
			if (packet.seqnum == base)
			{
				cout << endl << "[SUCCEED]: RECEIVER SEND BASE ACK, THEN RECEIVER CAN MOVE WINDOW!!!" << endl << endl;

				int move_number = 0;
				for (auto& pkt : Pkt_vec)
				{
					if (pkt.second == true)
					{
						move_number++;
						//当前base已经收到，可以将后面的一串true报文一起上交应用层
						Message msg;
						memcpy(msg.data, pkt.first.payload, sizeof(pkt.first.payload));
						pns->delivertoAppLayer(RECEIVER, msg);
						pkt.second = false;
					}
					else
						break;      //一碰到false的缓冲区域，立即break，接收方的窗口base移到此处	
				}

				//更新base到下一个fasle处
				base = (base + move_number) % max_seqnum;
				//更新buffernum
				buffernum -= move_number;
				//对于窗口内其他的不连续但已收到的分组，更新其在接收窗口内的位置（相当于移动接收方窗口）
				for (int i = 0; i < WindowLen; i++)
				{
					if (Pkt_vec[i].second == true && i >= move_number)
					{
						Pkt_vec[i - move_number].second = true;
						Pkt_vec[i - move_number].first = Pkt_vec[i].first;
						Pkt_vec[i].second = false;
					}
				}

				cout << endl << "------AFTER MOVING \"RECEIVER\" WINDOW------(A: Empty buffer; B: Mis_order and buffered packet; C:Unusable )" << endl;
				cout << "***RECEIVER WINDOW: [ ";
				//numberA-C分别是A类、B类和C类的报文个数
				for (int i = 0; i < WindowLen; i++)
				{
					if (Pkt_vec[i].second == false)
						cout << (base + i) % max_seqnum << "-A" << " ";     //输出A类和B类报文(其中细分是否buffered or not）
					else if (Pkt_vec[i].second == true)
						cout << (base + i) % max_seqnum << "-B" << " ";
				}
				cout << "]";
				for (int i = 0; i < max_seqnum - WindowLen; i++)            //输出C类报文
					cout << (base + WindowLen + i) % max_seqnum << "-C" << " ";
				cout << endl << endl;
			}
		}
		//2、当收到的数据包在接受窗口范围之前（对窗口和Pkt_Vec不造成影响)
		else if (Position(base, WindowLen, max_seqnum, packet.seqnum) == 0)
		{
			//直接确认，不做接收缓存
			//构造确认ackPkt
			Packet ackPkt;
			ackPkt.acknum = packet.seqnum;   //重复确认该数据包
			ackPkt.seqnum = -1;
			for (int i = 0; i < Configuration::PAYLOAD_SIZE; i++) {
				ackPkt.payload[i] = '.';
			}
			ackPkt.checksum = pUtils->calculateCheckSum(ackPkt);
			pUtils->printPacket("[SUCCEED]:RECEIVE PREV PACKET AND RETURN PREV ACK TO SENDER", ackPkt);
			pns->sendToNetworkLayer(SENDER, ackPkt);

			cout << endl << "------BEFORE MOVING \"RECEIVER\" WINDOW------(A: Empty buffer; B: Mis_order and buffered packet; C:Unusable )" << endl;
			cout << "***RECEIVER WINDOW: [ ";
			//numberA-C分别是A类、B类和C类的报文个数
			for (int i = 0; i < WindowLen; i++)
			{
				if (Pkt_vec[i].second == false)
					cout << (base + i) % max_seqnum << "-A" << " ";     //输出A类和B类报文(其中细分是否buffered or not）
				else if (Pkt_vec[i].second == true)
					cout << (base + i) % max_seqnum << "-B" << " ";
			}
			cout << "]";
			for (int i = 0; i < max_seqnum - WindowLen; i++)            //输出C类报文
				cout << (base + WindowLen + i) % max_seqnum << "-C" << " ";
			cout << endl << endl;
		}
	}
	else if (checkSum != packet.checksum) 
	{
		cout << endl;
		pUtils->printPacket("[FAILED]:PACKET CHECKSUM ERROR", packet);
		cout << endl;
	}
}