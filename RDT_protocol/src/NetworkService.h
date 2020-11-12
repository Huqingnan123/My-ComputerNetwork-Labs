#ifndef NETWORK_SERVICE_H
#define NETWORK_SERVICE_H

#include "RandomEventEnum.h"
#include "RdtSender.h"
#include "RdtReceiver.h"


//����NetworkService�����࣬�涨��ѧ��ʵ�ֵ�RdtSender��RdtReceiver���Ե��õĵĽӿڷ���
struct  NetworkService {

	virtual void startTimer(RandomEventTarget target, int timeOut,int seqNum) = 0;	//���ͷ������ʱ������RdtSender����
	virtual void stopTimer(RandomEventTarget target,int seqNum) = 0;				//���ͷ�ֹͣ��ʱ������RdtSender����
	virtual void sendToNetworkLayer(RandomEventTarget target, Packet pkt) = 0;		//�����ݰ����͵�����㣬��RdtSender��RdtReceiver����
	virtual void delivertoAppLayer(RandomEventTarget target, Message msg) = 0;		//�����ݰ����ϵݽ���Ӧ�ò㣬��RdtReceiver����

	virtual void init() = 0;														//��ʼ�����绷������main�����
	virtual void start() = 0;														//������绷������main�����
	virtual void setRtdSender(RdtSender *ps) = 0;									//���þ���ķ��ͷ�����
	virtual void setRtdReceiver(RdtReceiver *ps) = 0;								//���þ���ķ��ͷ�����
	virtual void setInputFile(const char *ifile) = 0;								//���������ļ�·��
	virtual void setOutputFile(const char *ofile) = 0;								//��������ļ�·��
	virtual ~NetworkService() = 0;
	virtual void setRunMode(int mode = 0) = 0;										//��������ģʽ��0��VERBOSEģʽ��1������ģʽ
};

#endif
