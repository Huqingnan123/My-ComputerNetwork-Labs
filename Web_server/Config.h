#pragma once
#include <string>
#include <winsock2.h>

//�Զ���������Ϣ
class Config
{
public:
	static std::string IP;              //��������ַ
	static int PORT;				    //�������˿�
	static std::string local_path;      //����Ŀ¼·��
private:
	Config(void);
	~Config(void);
};
