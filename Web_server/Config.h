#pragma once
#include <string>
#include <winsock2.h>

//自定义配置信息
class Config
{
public:
	static std::string IP;              //服务器地址
	static int PORT;				    //服务器端口
	static std::string local_path;      //本地目录路径
private:
	Config(void);
	~Config(void);
};
