#include "EasyTcpClient.hpp"
#include "Memory.hpp"
#include <vector>
void cmdThread(EasyTcpClient* etc)
{
	while (true)
	{
		char msgBuf[256] = {};
		scanf("%s", msgBuf);
		if (0 == strcmp(msgBuf, "exit"))
		{
			printf("client close\n");
			etc->Close();
			return;
		}
		else
		{
			etc->Send(msgBuf);
		}
	}
}
char* IP = (char *)"192.168.17.1";
int port = 3456;
int ClientNum = 4;
int ClientThreadNum = 4;
void sendThread(int id)
{
	std::vector<EasyTcpClient*> csock;
	int begin = id * ClientNum;
	int end = ClientNum * (id + 1);
	for (int i = begin; i < end; i++)
	{
		EasyTcpClient* client = new EasyTcpClient();
		client->Init();
		if (-1 != client->Connect(IP, port))
		{
			csock.emplace_back(client);
			client->Send((char*)"1");
			client->Start();
		}
	}
	for (;;)
	{
		for (auto client: csock)
		if (client != nullptr)
		{ 
			if (client->isRun()) client->Send((char*)"2");
			else
			{
				client = nullptr;
			     	
			}
		}
		Sleep(10);
	}
}
char* get_argstr(int argc, char* args[], int i, char* def)
{
	if (i >= argc)
	{
		printf("%d\n", argc);
		return def;
	}
	else return args[i];
}
int get_argint(int argc, char* args[], int i, int def)
{
	if (i >= argc && atoi(args[i]))
	{
		printf("%d\n", argc);
		return def;
	}
	else return atoi(args[i]);
}
int main(int argc,char* args[])
{
	IP = get_argstr(argc, args, 1, IP);
	port = get_argint(argc, args, 2, port);
	ClientNum = get_argint(argc, args, 3, ClientNum);
	ClientThreadNum = get_argint(argc, args, 4, ClientThreadNum);
	for (int i = 0; i < ClientThreadNum; i++)
	{
		std::thread t1(sendThread,i);
		t1.detach();
	}
	for (;;);

#ifdef _WIN32
	WSACleanup();
#endif
}