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
	const int N = 1000;
	EasyTcpClient* csock[N];
	for (int i = 0; i < N; i++)
	{
		csock[i] = new EasyTcpClient();
		csock[i]->Init();
		csock[i]->Connect((char*)"192.168.1.4", 3456);

	}
	for (;;)
	{
		for (int i = 0; i < N; i++)
		csock[i]->SendMessage((char*)"1");
	}
};