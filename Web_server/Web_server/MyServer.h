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
	int Winsock_Startup();		                       //��ʼ��Winsock
	int Server_Startup();                              //Socket��bind&Listen�ȹ���
	unsigned int Get_request_info(SOCKET* socket);     //��������֮�󣬽�����������Ӧ������ҳ����
	int Winsock_Cleanup();                             //���Winsock
private:
	SOCKET Listen_socket;                //�������˼����׽���
	struct sockaddr_in Server;
	struct sockaddr_in Client;           //IP�˿ڵ�ַ��Ϣ�洢�ṹ��
	int length;                          //sockaddr_in�ṹ��ĳ���
	std::string Client_IP;
	unsigned short Client_Port;          //Client IP and port
	std::string  method, url, file_type; //����ķ�����URL���ļ�����

	//judge file type using map(200 response)
	std::map<std::string, std::string> type_map{
	{ "html", "Content-Type: text/html;charset=UTF-8\r\n"}, { "jpg", "Content-Type: image/jpeg;charset=UTF-8\r\n" },
	{ "png", "Content-Type: image/png;charset=UTF-8\r\n"},  { "mp4", "Content-Type: video/mp4;charset=UTF-8\r\n" },
	{ "txt", "Content-Type: text/plain;charset=UTF-8\r\n"}, { "xml", "Content-Type: application/xml;charset=UTF-8\r\n" },
	{"ppt","Content-Type: application/vnd.ms-powerpoint;charset=UTF-8\r\n"} };
};