#pragma once

class WinsockEnv{
public:
	static int Startup();    //winsock��ʼ��
	static int Cleanup();    //���winsock
private:
	WinsockEnv(void);
	~WinsockEnv(void);
};
