#include "stdafx.h"
#include "Global.h"
#include "SRRdtSender.h"

SRRdtSender::SRRdtSender() :base(0), nextseqnum(0), WindowLen(4), max_seqnum(8)           
{
}


SRRdtSender::~SRRdtSender()
{
}



bool SRRdtSender::getWaitingState() 
{
	return ((nextseqnum - base + max_seqnum) % max_seqnum) >= WindowLen;
}




bool SRRdtSender::send(const Message& message) {
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

	Pkt_vec.push_back(make_pair(Send_pkt,false));	//加入到滑动窗口的数据包队列，初始状态false,未收到确认

	timerseq = Send_pkt.seqnum;     //当前数据报的计时器序号（都计时）
	pns->startTimer(SENDER, Configuration::TIME_OUT, timerseq);

	//调用模拟网络环境的sendToNetworkLayer，通过网络层发送到对方
	pns->sendToNetworkLayer(RECEIVER, Send_pkt);

	nextseqnum = (nextseqnum + 1) % max_seqnum;
	return true;
}

void SRRdtSender::receive(const Packet& ackPkt) {
	int checkSum = pUtils->calculateCheckSum(ackPkt);

	//当数据包没有损坏时
	if (checkSum == ackPkt.checksum)
	{
		//数据包的种类 (A: Send but not get ack; A*: Send and get ack; B: Usable but haven't sent; C:Unusable )
		int number_A, number_B, number_C;
		for (auto& pkt : Pkt_vec)
		{
			if (pkt.first.seqnum == ackPkt.acknum)
			{
				pkt.second = true;                        //对相应的数据包设置收到标志
				pns->stopTimer(SENDER, pkt.first.seqnum); //关闭相应的计时器
				cout << endl;
				pUtils->printPacket("[SUCCEED]:GET A ACK FROM RECEIVER", ackPkt);
				if (ackPkt.acknum != base)
					cout << "RECEIVER HAVE BUFFERED THIS PACKET..." << endl;
				cout << endl;
			}
		}

		cout << "------BEFORE MOVING \"SENDER\" WINDOW------(A: Send but not get ack; A*: Send and get ack; B: Usable but haven't sent; C:Unusable )" << endl;
		cout << "***SENDER WINDOW: [ ";
		number_A = (nextseqnum - base + max_seqnum) % max_seqnum;
		number_B = WindowLen - number_A;
		number_C = max_seqnum - WindowLen;                            //numberA-C分别是A类、B类和C类的报文个数
		for (int i = 0; i < number_A; i++)
		{
			if (Pkt_vec[i].second == false)
				cout << (base + i) % max_seqnum << "-A" << " ";      //输出A类报文(其中再细分是否收到ack，分为A1和A2)
			else if (Pkt_vec[i].second == true)
				cout << (base + i) % max_seqnum << "-A*" << " ";
		}
		for (int i = 0; i < number_B; i++)
			cout << (nextseqnum + i) % max_seqnum << "-B" << " ";     //输出B类报文
		cout << "]";
		for (int i = 0; i < number_C; i++)                            //输出C类报文
			cout << (base + WindowLen + i) % max_seqnum << "-C" << " ";
		cout << endl << endl;

		if (ackPkt.acknum == base)                        //ack等于base才移动窗口
		{
			cout << endl << "[SUCCEED]:RECEIVE BASE SEQNUM, THEN SENDER CAN MOVE WINDOW!!!" << endl << endl;

			//移动base到下一个标志为false的地方，跳过所有已记录过ack(true)的报文
			int move_number = 0; 
			for (auto& pkt : Pkt_vec)
			{
				if (pkt.second == true)
				{
					move_number++;
					pns->stopTimer(SENDER, pkt.first.seqnum); //关闭此计时器
				}
				else
					break;      //一碰到false的报文段，立即break，base移到此处	
			}

			base = (base + move_number) % max_seqnum;                       //更新base到下一个fasle处
			Pkt_vec.erase(Pkt_vec.begin(), Pkt_vec.begin() + move_number);  //把Pkt_vec前面已经标志为true的全部清除
			

			cout << "------After MOVING \"SENDER\" WINDOW------(A: Send but not get ack; A*: Send and get ack; B: Usable but haven't sent; C:Unusable )" << endl;
			cout << "***SENDER WINDOW: [ ";
			number_A = (nextseqnum - base + max_seqnum) % max_seqnum;
			number_B = WindowLen - number_A;
			number_C = max_seqnum - WindowLen;                            //numberA-C分别是A类、B类和C类的报文个数
			for (int i = 0; i < number_A; i++)
			{
				if (Pkt_vec[i].second == false)
					cout << (base + i) % max_seqnum << "-A" << " ";      //输出A类报文(其中再细分是否收到ack，分为A1和A2)
				else if (Pkt_vec[i].second == true)
					cout << (base + i) % max_seqnum << "-A*" << " ";
			}
			for (int i = 0; i < number_B; i++)
				cout << (nextseqnum + i) % max_seqnum << "-B" << " ";     //输出B类报文
			cout << "]";
			for (int i = 0; i < number_C; i++)                            //输出C类报文
				cout << (base + WindowLen + i) % max_seqnum << "-C" << " ";
			cout << endl << endl;
		}
	}
	else
		pUtils->printPacket("[FAILED]:ACKPKT CHECKSUM ERROR", ackPkt);
}

void SRRdtSender::timeoutHandler(int seqNum) {
	pns->stopTimer(SENDER, seqNum);
	for (auto& pkt : Pkt_vec)
	{
		if (pkt.first.seqnum == seqNum)
		{
			cout << endl;
			pUtils->printPacket("[TIMEOUT]: SEND THIS TIMEOUT PACKET AGAIN", pkt.first);
			cout << endl;
			pns->sendToNetworkLayer(RECEIVER, pkt.first);               //重新递交给网络层
			break;
		}
	}
	pns->startTimer(SENDER, Configuration::TIME_OUT, seqNum);   //重新开始计时
}
