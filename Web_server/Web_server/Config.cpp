#include "Config.h"
#include <string>


Config::Config(void)
{
}

Config::~Config(void)
{
}

std::string Config::IP = "127.0.0.1";	   //服务器IP地址
int Config::PORT = 7878;				   //服务器端口7878
std::string Config::local_path = "E:\\vs2019\\Computer-network\\Web_server\\Web_server";  //本地目录路径