//// SR.cpp : 定义控制台应用程序的入口点。
//
//#include "stdafx.h"
//#include "Global.h"
//#include "RdtSender.h"
//#include "RdtReceiver.h"
//#include "SRRdtSender.h"
//#include "SRRdtReceiver.h"
//
//int main(int argc, char* argv[])
//{
//	//如果需要使用其它的Rdt协议，只需要实例化其他具体Rdt实现类的实例，
//	//如GBNRdtSender和GBNRdtSeceiver
//	RdtSender* ps = new SRRdtSender();
//	RdtReceiver* pr = new SRRdtReceiver();
//	//pns->setRunMode(0); //VERBOS模式
//	pns->setRunMode(1); //安静模式
//	pns->init();
//	pns->setRtdSender(ps);
//	pns->setRtdReceiver(pr);
//	pns->setInputFile("E:\\vs2019\\Computer-network\\RDT_protocol\\input.txt");
//	pns->setOutputFile("E:\\vs2019\\Computer-network\\RDT_protocol\\output_SR.txt");
//	pns->start();
//	delete ps;
//	delete pr;
//	//指向唯一的工具类实例，只在main函数结束前delete
//	delete pUtils;
//	//指向唯一的模拟网络环境类实例，只在main函数结束前delete
//	delete pns;
//	return 0;
//};