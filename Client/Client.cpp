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
			etc->sendMessage(msgBuf);
			char recvmsg[256] = {};
			int nlen = etc->recieve(msgBuf, 256, 0);
			if (nlen <= 0)
			{
				printf("send message error\n");
			}
		}
	}
}
int main()
{
	EasyTcpClient etc = EasyTcpClient();
	etc.init();
	etc.Connect((char*)"192.168.1.3", 3456);
	std::thread cmdt(cmdThread, &etc);
	cmdt.detach();
	while (etc.isRun())
	{

	}
	etc.Close();
};