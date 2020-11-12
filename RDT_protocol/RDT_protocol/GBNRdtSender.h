#pragma once
#ifndef GBN_RDT_SENDER_H
#define GBN_RDT_SENDER_H
#include "stdafx.h"
#include "RdtSender.h"
#include <vector>

class GBNRdtSender :public RdtSender
{
private:
	int base;		                   //滑动窗口起始，最早未被确认或最初未发送的报文段
	int nextseqnum;			           //下一个发送序号
	int timerseq;		               //timer的序号
	const int WindowLen;		       //窗口的大小
    const int max_seqnum;              //编码报文序号的最大值（0-7循环 表示3位二进制数）
	std::vector<Packet> Pkt_vec;	   //窗口中的报文段队列

public:

	bool getWaitingState();
	bool send(const Message &message);	//发送应用层下来的Message，由NetworkServiceSimulator调用,如果发送方成功地将Message发送到网络层，返回true;如果因为发送方处于等待正确确认状态而拒绝发送Message，则返回false
	void receive(const Packet &ackPkt);	//接受确认Ack，将被NetworkServiceSimulator调用	
	void timeoutHandler(int seqNum);	//Timeout handler，将被NetworkServiceSimulator调用

public:
	GBNRdtSender();
	virtual ~GBNRdtSender();
};

#endif

