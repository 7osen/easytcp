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
	std::thread cmdt(cmdThread, &ets);
	ets.Start();
	cmdt.detach();
	ets.Run();
	if (ets.isRun()) ets.Close();
	while (1);
	return 0;
}