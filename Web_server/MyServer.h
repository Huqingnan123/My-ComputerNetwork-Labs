#pragma once
#include <WinSock2.h>
#include <string>
#include <map>

#define MAXCONNECT 5
#define BUF_LEN 65535

class MyServer {
public:
	MyServer(void) = default;
	~MyServer(void) = default;
	int Winsock_Startup();		                       //初始化Winsock
	int Server_Startup();                              //Socket的bind&Listen等工作
	unsigned int Get_request_info(SOCKET* socket);     //接受请求之后，解析报文与响应呈现网页内容
	int Winsock_Cleanup();                             //清除Winsock
private:
	SOCKET Listen_socket;                //服务器端监听套接字
	struct sockaddr_in Server;
	struct sockaddr_in Client;           //IP端口地址信息存储结构体
	int length;                          //sockaddr_in结构体的长度
	std::string Client_IP;
	unsigned short Client_Port;          //Client IP and port
	std::string  method, url, file_type; //请求的方法，URL和文件类型

	//judge file type using map(200 response)
	std::map<std::string, std::string> type_map{
	{ "html", "Content-Type: text/html;charset=UTF-8\r\n"}, { "jpg", "Content-Type: image/jpeg;charset=UTF-8\r\n" },
	{ "png", "Content-Type: image/png;charset=UTF-8\r\n"},  { "mp4", "Content-Type: video/mp4;charset=UTF-8\r\n" },
	{ "txt", "Content-Type: text/plain;charset=UTF-8\r\n"}, { "xml", "Content-Type: application/xml;charset=UTF-8\r\n" },
	{"ppt","Content-Type: application/vnd.ms-powerpoint;charset=UTF-8\r\n"} };
};