#include "WinsockEnv.h"
#include <iostream>
#include <winsock2.h>
#pragma comment(lib, "Ws2_32.lib")

#define MAJORVERSION 2  //Winsock���汾��
#define MINORVERSION 2	//Winsock�ΰ汾��

WinsockEnv::WinsockEnv(void)
{
}

WinsockEnv::~WinsockEnv(void)
{
}

int WinsockEnv::Startup() {

	WSADATA wsaData;	//���ڷ���Winsock�Ļ�����Ϣ   

	//��ʼ�� winsock, �������ֵ����0����Winsock��ʼ��ʧ��
	if (WSAStartup(MAKEWORD(MAJORVERSION, MINORVERSION), &wsaData)) { 
		std::cout << "Failed to initialize Winsock..." << std::endl << std::endl;
		return -1;
	}

	//�жϷ��ص�Winsock�汾��
	if (LOBYTE(wsaData.wVersion) != MAJORVERSION || HIBYTE(wsaData.wVersion) != MINORVERSION) { 
		WSACleanup();  
		std::cout << "Winsock version error!" << std::endl;
		return -1;
	}

	//�ɹ���ʼ��
	std::cout << "Succeed to initialize Winsock..." << std::endl << std::endl;
	return 0;
}

int WinsockEnv::Cleanup() {
	if (WSACleanup())
	{
		std::cout << "Succeed to clean up Winsock..." << std::endl << std::endl;
		return 0;
	}
	return -1;
}