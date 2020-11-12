#include "stdafx.h"
#include "Global.h"
#include "GBNRdtSender.h"
#include <string.h>

GBNRdtSender::GBNRdtSender() :base(0), nextseqnum(0), WindowLen(4), max_seqnum(8)
{
}


GBNRdtSender::~GBNRdtSender()
{
}


//当窗口WindowLen内的pkt都等待接收ack，此时waitingstate = 1
bool GBNRdtSender::getWaitingState() {
	return ((nextseqnum - base + max_seqnum) % max_seqnum) >= WindowLen;
}



//应用层调用GBNRdtSender的send函数发送数据
bool GBNRdtSender::send(const Message& message) {
	if (this->getWaitingState()) { //发送方处于等待确认状态
		return false;
	}

	//构造要发送的报文段
	Packet& Send_pkt = *(new Packet);
	Send_pkt.seqnum = this->nextseqnum;
	Send_pkt.acknum = -1;                                               //忽略该字段
	memcpy(Send_pkt.payload, message.data, sizeof(message.data));		//报文段数据
	Send_pkt.checksum = pUtils->calculateCheckSum(Send_pkt);
	cout << endl;
	pUtils->printPacket("[SUCCEED]:SEND PACKETS", Send_pkt);
	cout << endl;

	Pkt_vec.push_back(Send_pkt);	//加入到滑动窗口的数据包队列，初始状态为“可用，还未发送”

	//当前队列中没有发送还未确认的, 启动发送方定时器（基于baseseq,说明base当前还未收到ack)
	if (base == nextseqnum)
	{
		timerseq = base;
		pns->startTimer(SENDER, Configuration::TIME_OUT, timerseq);
	}

	//调用模拟网络环境的sendToNetworkLayer，通过网络层发送到对方
	pns->sendToNetworkLayer(RECEIVER, Send_pkt);

	nextseqnum = (nextseqnum + 1) % max_seqnum;
	return true;
}

//接收来自低层的确认ack
void GBNRdtSender::receive(const Packet& ackPkt) {
	int checkSum = pUtils->calculateCheckSum(ackPkt);

	if (checkSum == ackPkt.checksum)
	{
		int number_A, number_B, number_C;
		cout << endl;
		pUtils->printPacket("[SUCCEED]:GET ACK FROM RECEIVER AND MOVE WINDOW", ackPkt);
		cout << endl;
		cout << "--------BEFORE MOVING WINDOW-------- (A: Send but not get ack;  B: Usable but haven't sent;  C:Unusable )" << endl;
		cout << "[ ";
		number_A = (nextseqnum - base + max_seqnum) % max_seqnum;
		number_B = WindowLen - number_A;
		number_C = max_seqnum - WindowLen;                          //numberA-C分别是A类、B类和C类的报文个数
		for (int i = 0; i < number_A; i++)
		{
			cout << (base + i) % max_seqnum << "-A" << " ";           //输出A类报文
		}
		for (int i = 0; i < number_B; i++)
			cout << (nextseqnum + i) % max_seqnum << "-B" << " ";     //输出B类报文
		cout << "]";
		for (int i = 0; i < number_C; i++)                          //输出C类报文
			cout << (base + WindowLen + i) % max_seqnum << "-C" << " ";
		cout << endl << endl;

		//acknum只有base和lastpkt->acknum两种可能，GBNSENDER窗口要么移动一格，要么不移动等超时重传！！！
		//只有当ackpkt.acknum == base的时候才移动base，更新Pkt_vec数据报vec
		//move_number要么是1，要么是0
		int move_number = (ackPkt.acknum - base + 1 + max_seqnum) % max_seqnum;
		Pkt_vec.erase(Pkt_vec.begin(), Pkt_vec.begin() + move_number);

		base = (ackPkt.acknum + 1) % max_seqnum;	//更新移动base，只有acknum == base才会移动窗口
		cout << "--------After MOVING WINDOW-------- (A: Send but not get ack;  B: Usable but haven't sent;  C:Unusable )" << endl;
		cout << "[ ";
		number_A = (nextseqnum - base + max_seqnum) % max_seqnum;
		number_B = WindowLen - number_A;
		number_C = max_seqnum - WindowLen;                          //numberA-C分别是A类、B类和C类的报文个数
		for (int i = 0; i < number_A; i++)
		{
			cout << (base + i) % max_seqnum << "-A" << " ";           //输出A类报文
		}
		for (int i = 0; i < number_B; i++)
			cout << (nextseqnum + i) % max_seqnum << "-B" << " ";     //输出B类报文
		cout << "]";
		for (int i = 0; i < number_C; i++)                          //输出C类报文
			cout << (base + WindowLen + i) % max_seqnum << "-C" << " ";
		cout << endl << endl;

		//此时没有已发送但待确认的报文段，关闭定时器, 此时发送的全部被接受(GBN只有一个计时器！！！）
		if (base == nextseqnum)
			pns->stopTimer(SENDER, timerseq);
		else
		{
			pns->stopTimer(SENDER, timerseq);
			//重启计时器		
			timerseq = base;
			pns->startTimer(SENDER, Configuration::TIME_OUT, timerseq);
		}
	}
	else
		pUtils->printPacket("[FAILED]:ACKPKT CHECKSUM ERROR", ackPkt);
}

void GBNRdtSender::timeoutHandler(int seqNum) {
	cout << endl;
	pUtils->printPacket("[TIMEOUT]: SEND PACKETS AFTER BASE AGAIN", this->Pkt_vec[0]);
	cout << endl;
	pns->stopTimer(SENDER, seqNum);
	if (base != nextseqnum)	//窗口中有之前已发送，此时需要重发base之后的所有报文，重启计时器
	{
		pns->startTimer(SENDER, Configuration::TIME_OUT, seqNum);
		for (auto& i : Pkt_vec)
		{
			pns->sendToNetworkLayer(RECEIVER, i);    //重新发送
		}
	}
}
