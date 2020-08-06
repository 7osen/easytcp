#include "EasyTcpClient.hpp"
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
			etc->SendMessage(msgBuf);
		}
	}
}
int main()
{
	EasyTcpClient* csock[1000];
	for (int i = 0; i < 1000; i++)
	{
		csock[i] = new EasyTcpClient();
		csock[i]->Init();
		csock[i]->Connect((char*)"192.168.1.4", 3456);
		csock[i]->SendMessage((char*)"hello");
	}
	EasyTcpClient etc = EasyTcpClient();
	etc.Init();
	etc.Connect((char*)"192.168.1.4", 3456);
	std::thread cmdt(cmdThread, &etc);
	cmdt.detach();
	while (etc.isRun())
	{
		etc.Run();
	}
	etc.Close();
};