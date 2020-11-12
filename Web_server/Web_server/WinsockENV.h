#pragma once

class WinsockEnv{
public:
	static int Startup();    //winsock³õÊ¼»¯
	static int Cleanup();    //Çå³ýwinsock
private:
	WinsockEnv(void);
	~WinsockEnv(void);
};
