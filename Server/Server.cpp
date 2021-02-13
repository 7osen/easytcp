#include "EasyTcpServer.hpp"
#include <thread>
void cmdThread(EasyTcpServer* ets)
{
	while (ets->isRun())
	{
		char cmd[256] = {};
		scanf("%s", cmd);
		if (!strcmp(cmd, "exit"))
		{
			ets->Close();
		}
		else
		{
			printf("invalid command!\n");
		}
	}
}
char* get_argstr(int argc, char* args[], int i,char* def)
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
int main(int argc, char* args[])
{

	char* IP = nullptr;
	
	int port = 3456;

	//IP = get_argstr(argc, args, 1,IP);
	port = get_argint(argc,args,1,port);
	EasyTcpServer ets = EasyTcpServer();
	ets.init();
	ets.Bind(IP, port);
	ets.Listen();
	std::thread cmdt(cmdThread, &ets);
	ets.Start<CellSelect>();
	cmdt.detach();
	ets.Run();
	if (ets.isRun()) ets.Close();
	while (1);
	return 0;
}
