#pragma once
#ifndef SR_RDT_RECEIVER_H
#define SR_RDT_RECEIVER_H
#include "stdafx.h"
#include "RdtReceiver.h"
#include <vector>
#include <utility>

class SRRdtReceiver :public RdtReceiver
{
private:
	int base;                       //窗口起始
	int buffernum;                  //已经缓存的数据包个数
	const int WindowLen;		    //窗口的大小
	const int max_seqnum;           //编码报文序号的最大值（0-7循环 表示3位二进制数）
	std::vector<std::pair<Packet,bool> > Pkt_vec;    //缓存窗口序列

public:
	SRRdtReceiver();
	virtual ~SRRdtReceiver();

public:

	void receive(const Packet& packet);	//接收报文，将被NetworkService调用
};

#endif

