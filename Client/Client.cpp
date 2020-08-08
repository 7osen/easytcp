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
const int N = 500;
EasyTcpClient* csock[4*N];
void sendThread(int id)
{
	int begin = id * N;
	int end = N * (id + 1);
	for (int i = begin; i < end; i++)
	{
		csock[i] = new EasyTcpClient();
		csock[i]->Init();
		csock[i]->Connect((char*)"192.168.1.4", 3456);

	}
	for (;;)
	{
		for (int i = begin; i < end; i++)
			csock[i]->SendMessage((char*)"1");
	}
}
int main()
{
	for (int i = 0; i < 4; i++)
	{
		std::thread t1(sendThread,i);
		t1.detach();
	}
	for (;;);
};