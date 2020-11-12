#include "MyServer.h"
#include "WinsockENV.h"

int main()
{
	MyServer server;
	if (server.Winsock_Startup() == -1)
		return -1;
	else if (server.Server_Startup() == -1)
		return -1;
	else if (server.Winsock_Cleanup() == -1)
		return -1;
	return 0;
}