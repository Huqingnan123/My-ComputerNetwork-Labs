#include <iostream>
#include <string>
#include <fstream>
#include <istream>
#include <algorithm>
#include <process.h>
#include <direct.h>
#include <io.h>
#include <conio.h>
#include "MyServer.h"
#include "WinsockEnv.h"
#include "Config.h"

#pragma comment(lib, "Ws2_32.lib")
#pragma warning(disable:4996)

char rcv_Buf[BUF_LEN];         //接受Client请求报文

//初始化Winsock
int MyServer::Winsock_Startup() {

	if (WinsockEnv::Startup() == -1) 
		return -1;	//初始化Winsock
	return 0;
}

//WebServer服务启动
int MyServer::Server_Startup()
{
	char ch;
	while (1)
	{
		std::cout << "Press \"y\" to start the server or \"n\" to quit..." << std::endl;
		std::cin >> ch;
		switch (ch)
		{
		case 'y':
		{
			//stream socket
			Listen_socket = socket(AF_INET, SOCK_STREAM, 0);
			if (Listen_socket == INVALID_SOCKET)
			{
				std::cout << "Failed to initialize Listen_socket..." << std::endl;
				return -1;
			}
			std::cout << "Succeed to initialize Listen_socket..." << std::endl;

			//Config IP and Port
			std::cout << std::endl << "Configure the IP and Port by Config-file or not (default)? (Y or N) : ";
			char choice;
		Input: std::cin >> choice;
			if (choice == 'N')
			{
				std::cout << std::endl;
				Server.sin_family = AF_INET;
				Server.sin_port = htons(5050);       //默认端口5050
				Server.sin_addr.s_addr = htonl(INADDR_ANY);
			}
			else if (choice == 'Y')
			{
				Server.sin_family = AF_INET;
				Server.sin_port = htons(Config::PORT);
				Server.sin_addr.s_addr = inet_addr(Config::IP.data());
				std::cout << std::endl << "----------------------------------------------------------" << std::endl;
				std::cout << "                   IP:   " << Config::IP.data() << std::endl;
				std::cout << "                   Port:    " << Config::PORT << std::endl;
				std::cout << "----------------------------------------------------------" << std::endl << std::endl;
			}
			else
			{
				std::cout << "Input your choice again (Y or N): ";
				goto Input;
			}

			//bind
			if (bind(Listen_socket, (LPSOCKADDR)&Server, sizeof(Server)))
			{
				std::cout << "Failed to bind the socket with IP&Port..." << std::endl;
				closesocket(Listen_socket);
				WSACleanup();
				return -1;
			}
			std::cout << "Succeed to bind the socket with IP&Port..." << std::endl;

			//listen
			if (listen(Listen_socket, MAXCONNECT) == SOCKET_ERROR)
			{
				std::cout << "Failed to listen this socket...\n" << std::endl;
				closesocket(Listen_socket);
				WSACleanup();
				return -1;
			}
			std::cout << "Succeed to listen this socket...\n" << std::endl;
			std::cout << "----------------------------------------------------------\n" << std::endl;

			//settings of Blockmode
			u_long Blockmode = 1;
			if (ioctlsocket(Listen_socket, FIONBIO, &Blockmode) == SOCKET_ERROR) {
				std::cout << "Setting blockmode Error!" << std::endl;
				return -1;
			}

			SOCKET Accept_socket;
			while (1) {
				if (kbhit())
					break;

				length = sizeof(Client);
				Accept_socket = accept(Listen_socket, (struct sockaddr*)&Client, &length);
				if (Accept_socket == INVALID_SOCKET)
				{
					if (WSAGetLastError() == 10035)
						continue;
					else
					{
						std::cout << "Failed to accept from client! Error..." << std::endl;
						break;
					}
				}

				//get info of messages
				Get_request_info(&Accept_socket);
			}
			if (Listen_socket != NULL)
				closesocket(Listen_socket);
			std::cout << "Already close the server..." << std::endl << std::endl;
			break;
		}
		case 'n':
		{
			return 0;
		}
		}
	}
}

//解析请求报文, 给Client发送文件内容, 响应
unsigned int MyServer::Get_request_info(SOCKET* socket) {
	SOCKET Accept_socket = *socket;
	memset(rcv_Buf, 0, sizeof(rcv_Buf));

	fd_set readfds;
	timeval time_out;
	FD_ZERO(&readfds);
	FD_SET(Accept_socket, &readfds);
	//程序会停在select这里等待，直到被监视的一个socket发生了状态改变。
	//当句柄的状态变成可读的时侯系统就会告诉select函数返回
	//select函数与一个非阻塞态的socket连用的时候，select可以起到监视端口是否可用的作用
	//如果端口不可用，直接返回错误信息，而不需要再去recv, 或者send了
	if (select(Accept_socket + 1, &readfds, NULL, NULL, &time_out) != 1)
		Sleep(200);

	//Client requesting messages, recieved by Server
	if (recv(Accept_socket, rcv_Buf, sizeof(rcv_Buf), 0) == SOCKET_ERROR)
	{
		if (WSAGetLastError() == 10035)
			;
		else
			std::cout << "Failed to recieve messages from Client:" << WSAGetLastError() << std::endl << std::endl;
		//error, close it
		closesocket(Accept_socket);
		return -1;
	}

	//Client IP and port
	Client_IP = inet_ntoa(Client.sin_addr);
	std::cout << "Client_IP: " << Client_IP << std::endl;
	Client_Port = Client.sin_port;
	std::cout << "Client_Port: " << Client_Port << std::endl << std::endl;

	//recieved by server successfully
	std::cout << "Received requesting messages from Client: \n\n" << rcv_Buf << std::endl;

	//get requesting method（save request messages in recieved)
	std::string received(rcv_Buf);      
	int space_pos = received.find(" ");
	method = received.substr(0, space_pos);
	std::cout << "Requesting method is: " << method << std::endl;

	std::string later_info = received.substr(space_pos + 1, received.length());

	//get requesting url
	space_pos = later_info.find(" ");
	url = later_info.substr(0, space_pos);
	std::cout << "Requesting url is: " << url << std::endl;
	int dot_pos = url.find(".");

	//get requesting file_type
	file_type = url.substr(dot_pos + 1, url.length() - dot_pos - 1);
	std::cout << "Requesting file type is: " << file_type << std::endl;

	//get file path
	std::string file_path(Config::local_path.data());
	file_path.append(url.replace(0, 1, "\\"));
	std::cout << "The local path of the file requested is: " << file_path << std::endl;

	FILE* fp = fopen(file_path.c_str(), "rb");

	// if don't have this file, send "404-file not found", close current Accept_socket
	if (fp == NULL)
	{
		std::string message_404 = "HTTP/1.1 404 NOT FOUND\r\nConnection: keep-alive\r\nServer:Net_Server\r\nContent-Type: text/html\r\n\r\n";
		send(Accept_socket, message_404.data(), message_404.length(), 0);

		//if method is get, send html_404
		if (strcmp(method.c_str(), "GET") == 0)
		{
			std::string str, Buf_404;
			std::ifstream html_404;
			html_404.open("404.TXT");
			if (html_404.is_open())
			{
				while (std::getline(html_404, str))
					Buf_404.append(str);
				html_404.close();
			}
			send(Accept_socket, Buf_404.data(), Buf_404.length(), 0);
		}
		closesocket(Accept_socket);
		std::cout << "Requesting file is not existed...\n" << std::endl;
		std::cout << "----------------------------------------------------------" << std::endl << std::endl;
		return -1;
	}

	//file length
	int file_len = filelength(fileno(fp));
	std::cout << "The requested file's length is: " << file_len << std::endl << std::endl;

	// 200 response
	std::string Buf_200 = "HTTP/1.1 200 OK\r\nConnection: keep-alive\r\nServer:Net_Server\r\nContent-Length: +" + std::to_string(file_len) + "\r\n";

	Buf_200 += type_map[file_type];
	Buf_200 += "\r\n";
	send(Accept_socket, Buf_200.data(), Buf_200.length(), 0);
	std::cout << Buf_200 << std::endl;

	//if method is get, send relevent html info
	if (strcmp(method.c_str(), "GET") == 0)
	{
		char* Send_buf = new char[file_len + 1];
		memset(Send_buf, 0, file_len + 1);
		fread(Send_buf, sizeof(char), file_len, fp);
		if (send(Accept_socket, Send_buf, file_len, 0) == SOCKET_ERROR)
			std::cout << "Failed to send file requested: " << WSAGetLastError() << std::endl;
		else
		{
			std::cout << "Succeed to send file requested..." << std::endl << std::endl;
			std::cout << "----------------------------------------------------------" << std::endl << std::endl;
		}
	}
	fclose(fp);
	closesocket(Accept_socket);
	return 0;
}

//清除Winsock
int MyServer::Winsock_Cleanup() {

	if (WinsockEnv::Cleanup() == -1)
		return -1;	
	return 0;
}