#include "WinsockEnv.h"
#include <iostream>
#include <winsock2.h>
#pragma comment(lib, "Ws2_32.lib")

#define MAJORVERSION 2  //Winsock主版本号
#define MINORVERSION 2	//Winsock次版本号

WinsockEnv::WinsockEnv(void)
{
}

WinsockEnv::~WinsockEnv(void)
{
}

int WinsockEnv::Startup() {

	WSADATA wsaData;	//用于返回Winsock的环境信息   

	//初始化 winsock, 如果返回值不是0，则Winsock初始化失败
	if (WSAStartup(MAKEWORD(MAJORVERSION, MINORVERSION), &wsaData)) { 
		std::cout << "Failed to initialize Winsock..." << std::endl << std::endl;
		return -1;
	}

	//判断返回的Winsock版本号
	if (LOBYTE(wsaData.wVersion) != MAJORVERSION || HIBYTE(wsaData.wVersion) != MINORVERSION) { 
		WSACleanup();  
		std::cout << "Winsock version error!" << std::endl;
		return -1;
	}

	//成功初始化
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