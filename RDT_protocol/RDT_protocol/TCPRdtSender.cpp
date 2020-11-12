#include "stdafx.h"
#include "Global.h"
#include "TCPRdtSender.h"
#include <string.h>

//TCP的ACKnum是期待收到的下一个数据包的seqnum
int need_Redundant(int base, int WindowLen, int max_seqnum, int seq) {
	//当接受窗口连续时（不经过max_seqnum-1到0的区间）
	if (base < (base + WindowLen) % max_seqnum)
	{
		if (seq > base)
			return false;
		else
			return true;
	}
	//当接受窗口不连续时（中间经过max_seqnum-1到0的区间，一次循环往复）
	else
	{
		if (seq <= base && seq > (base + WindowLen) % max_seqnum)
			return true;
		else
			return false;
	}
}

TCPRdtSender::TCPRdtSender() :base(0), nextseqnum(0), Redundant_ACK(0), WindowLen(4), max_seqnum(8)
{
}


TCPRdtSender::~TCPRdtSender()
{
}


//当窗口WindowLen内的pkt都等待接收ack，此时waitingstate = 1
bool TCPRdtSender::getWaitingState() {
	return ((nextseqnum - base + max_seqnum) % max_seqnum) >= WindowLen;
}



//应用层调用TCPRdtSender的send函数发送数据
bool TCPRdtSender::send(const Message& message) {
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
void TCPRdtSender::receive(const Packet& ackPkt) {
	int checkSum = pUtils->calculateCheckSum(ackPkt);

	if (checkSum == ackPkt.checksum)
	{
		if (!need_Redundant(base, WindowLen, max_seqnum, ackPkt.acknum))
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


			//移动base，更新Pkt_vec数据报vec
			int move_number = (ackPkt.acknum - base + max_seqnum) % max_seqnum;
			Pkt_vec.erase(Pkt_vec.begin(), Pkt_vec.begin() + move_number);

			base = (ackPkt.acknum) % max_seqnum;	//更新移动base(TCP的ack是下一个期望收到的序列号）
			Redundant_ACK = 0;
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

			//此时没有已发送但待确认的报文段，关闭定时器, 此时发送的全部被接受(TCP只有一个计时器！！！）
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
		{
			Redundant_ACK++;    //冗余ack个数为3时，快速重传
			if (Redundant_ACK == 3)
			{
				cout << endl << "[3 ACK]:THREE REDUNDANT ACK ALREADY" << endl << endl;
				//快速重传开始(重传冗余ACK序号的对应数据包）
				for (auto& pkt : Pkt_vec)
				{
					if (pkt.seqnum == ackPkt.acknum)
					{
						pns->sendToNetworkLayer(RECEIVER, pkt);    //快速重传
						pUtils->printPacket("[SUCCEED]:FAST RETRANSIMIT FINISHED!", pkt);
						break;
					}
				}
				Redundant_ACK = 0;
			}
		}
	}
	else
		pUtils->printPacket("[FAILED]:ACKPKT CHECKSUM ERROR", ackPkt);
}

void TCPRdtSender::timeoutHandler(int seqNum) {
	cout << endl;
	pUtils->printPacket("[TIMEOUT]:ONLY RESEND BASE PACKETS", this->Pkt_vec[0]);
	cout << endl;
	Redundant_ACK = 0;      //重置冗余计数为0
	if (base != nextseqnum)	//窗口中有之前已发送，此时需要重发base，关闭base计时器，重启之后数据包的计时器
	{
		pns->stopTimer(SENDER, seqNum);
		pns->sendToNetworkLayer(RECEIVER, Pkt_vec[0]);
		pns->startTimer(SENDER, Configuration::TIME_OUT, seqNum);
	}
	else
	{
		//仅把base计时器关闭即可
		pns->stopTimer(SENDER, seqNum);
	}
}
