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
int main()
{
	EasyTcpServer ets = EasyTcpServer();
	ets.init();
	ets.Bind("192.168.1.3", 3456);
	ets.Listen();
	std::thread cmdt(cmdThread, &ets);
	cmdt.detach();
	ets.Run();
	ets.Close();
	return 0;
}