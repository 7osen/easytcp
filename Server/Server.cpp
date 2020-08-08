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
	ets.Bind(nullptr, 3456);
	ets.Listen();
	ets.Start();
	std::thread cmdt(cmdThread, &ets);
	cmdt.detach();
	while (ets.isRun()) ets.Run();
	ets.Close();
	return 0;
}