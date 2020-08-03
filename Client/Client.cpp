#define WIN32_LEAN_AND_MEAN
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#ifdef _WIN32
#include <Windows.h>
#include <WinSock2.h>
#else
#include <unistd.h>
#include <arpa/inet.h>
#include <string.h>
#define SOCKET int
#define INVALID_SOCKET  (SOCKET)(~0)
#define SOCKET_ERROR            (-1)
#endif

#include <stdio.h>
#include <thread>

#pragma comment(lib, "ws2_32.lib")
bool _TRun = true;
void cmdThread(SOCKET _sock)
{
	while (true)
	{
		char msgBuf[256] = {};
		scanf("%s", msgBuf);
		if (0 == strcmp(msgBuf, "exit"))
		{
			printf("client close\n");
			_TRun = false;
			return;
		}
		else
		{
			send(_sock, msgBuf, strlen(msgBuf) + 1, 0);
			char recvmsg[256] = {};
			int nlen = recv(_sock, recvmsg, 256, 0);
			if (nlen > 0)
			{
				printf("receive message: %s\n", recvmsg);
			}
			else
			{
				printf("send message error\n");
			}
		}
	}
}
int main()
{
	//windows socket 2.x 环境
#ifdef _WIN32
	WORD ver = MAKEWORD(2, 2);
	WSADATA dat;
	WSAStartup(ver, &dat);
#endif
	//建立socket
	SOCKET _sock = socket(AF_INET, SOCK_STREAM, 0);
	if (SOCKET_ERROR == _sock)
	{
		printf("build socket error!\n");
	}
	else
	{
		printf("build socket success!\n");
	}
	//连接服务器
	sockaddr_in _sin = {};
	_sin.sin_family = AF_INET;
	_sin.sin_port = htons(3456);
#ifdef _WIN32
	_sin.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
#else 
	_sin.sin_addr.s_addr = inet_addr("192.168.17.1");
#endif
	int retc = connect(_sock, (sockaddr*)& _sin, sizeof(sockaddr_in));
	if (SOCKET_ERROR == retc)
	{
		printf("connect error!\n");
	}
	else
	{
		printf("connect success!\n");
	}
	std::thread cmdt(cmdThread, _sock);
	cmdt.detach();
	while (_TRun)
	{

	}
#ifdef _WIN32
	closesocket(_sock);
	WSACleanup();
#else 
	close(_sock);
#endif
	getchar();
	return 0;
}