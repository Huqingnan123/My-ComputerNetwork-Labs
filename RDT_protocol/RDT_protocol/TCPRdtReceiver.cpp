#include "stdafx.h"
#include "Global.h"
#include "TCPRdtReceiver.h"
#define max_seqnum 8

TCPRdtReceiver::TCPRdtReceiver() :expectSequenceNumberRcvd(0)
{
	//初始状态下，上次发送的确认包的确认序号为-1，
	//使得当第一个接受的数据包出错时该确认报文的确认号为-1
	lastAckPkt.acknum = -1;
	lastAckPkt.checksum = 0;
	lastAckPkt.seqnum = -1;	     //忽略该字段
	for (int i = 0; i < Configuration::PAYLOAD_SIZE; i++) {
		lastAckPkt.payload[i] = '.';
	}
	lastAckPkt.checksum = pUtils->calculateCheckSum(lastAckPkt);
}


TCPRdtReceiver::~TCPRdtReceiver()
{
}

void TCPRdtReceiver::receive(const Packet& packet) {
	//检查校验和是否正确
	int checkSum = pUtils->calculateCheckSum(packet);
	//如果校验和正确，同时收到报文的序号等于接收方期待收到的报文序号一致
	if (checkSum == packet.checksum && this->expectSequenceNumberRcvd == packet.seqnum) {
		cout << endl;
		pUtils->printPacket("[SUCCEED]:RECEIVE PACKET", packet);
		cout << endl;
		//取出Message，向上递交给应用层
		Message msg;
		memcpy(msg.data, packet.payload, sizeof(packet.payload));
		pns->delivertoAppLayer(RECEIVER, msg);

		lastAckPkt.acknum = (packet.seqnum + 1) % max_seqnum; //确认序号等于期待收到的下一个报文序号！！！

		lastAckPkt.checksum = pUtils->calculateCheckSum(lastAckPkt);
		cout << endl;
		pUtils->printPacket("[SUCCEED]:RETURN RIGHT ACK TO SENDER", lastAckPkt);
		cout << endl;
		pns->sendToNetworkLayer(SENDER, lastAckPkt);	//调用模拟网络环境的sendToNetworkLayer，通过网络层发送确认报文到对方

		this->expectSequenceNumberRcvd = (this->expectSequenceNumberRcvd + 1) % max_seqnum; //接收序号在0-7之间循环
	}
	else {
		if (checkSum != packet.checksum) {
			cout << endl;
			pUtils->printPacket("[FAILED]:PACKET CHECKSUM ERROR", packet);
			cout << endl;
		}
		else {
			cout << endl;
			pUtils->printPacket("[FAILED]:PACKET SEQNUM ERROR", packet);
			cout << endl;
		}
		//累计确认，继续发送上一次的acknum
		cout << endl;
		pUtils->printPacket("[REDUNDANT]:RETURN PREV_SUCCEED ACK TO SENDER", lastAckPkt);
		cout << endl;
		pns->sendToNetworkLayer(SENDER, lastAckPkt);	//调用模拟网络环境的sendToNetworkLayer，通过网络层发送上次的确认报文
		//相当于发送一个冗余ack(TCP中）
	}
}