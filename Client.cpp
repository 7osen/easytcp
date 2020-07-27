#define WIN32_LEAN_AND_MEAN
#define _WINSOCK_DEPRECATED_NO_WARNINGS


#include <Windows.h>
#include <WinSock2.h>
#include <stdio.h>

#pragma comment(lib, "ws2_32.lib")
int main()
{
	//windows socket 2.x 环境
	WORD ver = MAKEWORD(2, 2);
	WSADATA dat;
	WSAStartup(ver, &dat);
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
	_sin.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
	int retc = connect(_sock, (sockaddr*)& _sin, sizeof(sockaddr_in));
	if (SOCKET_ERROR == retc)
	{
		printf("connect error!\n");
	}
	else
	{
		printf("connect success!\n");
	}
	while (true)
	{
		char msgBuf[256] = {};
		scanf("%s", msgBuf);
		if (0 == strcmp(msgBuf, "exit"))
		{
			printf("client close\n");
			break;
		}
		else if (0 == strcmp(msgBuf, "getIP"))
		{
			printf("%s\n", msgBuf);
			send(_sock, msgBuf, strlen(msgBuf) + 1, 0);
			char recvmsg[256] = {};
			int nlen = recv(_sock, recvmsg, 256, 0);
			if (nlen > 0)
			{
				printf("receive message: %s\n", recvmsg);
			}
		}
	}
	closesocket(_sock);
	WSACleanup();
	getchar();
	return 0;
}