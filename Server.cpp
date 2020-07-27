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
	SOCKET _sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	//Bind绑定端口
	sockaddr_in _sin = {};
	_sin.sin_family = AF_INET;
	_sin.sin_port = htons(3456);
	_sin.sin_addr.S_un.S_addr = INADDR_ANY;
	int retbind = bind(_sock, (sockaddr*)& _sin, sizeof(_sin));
	if (SOCKET_ERROR == retbind)
	{
		printf("bind error!\n");
	}
	else
	{
		printf("bind success!\n");
	}
	//listen 监听端口
	int retlis = listen(_sock, 5);
	if (SOCKET_ERROR == retlis)
	{
		printf("listen error!\n");
	}
	else
	{
		printf("listen success!\n");
	}
	while (true)
	{
		sockaddr_in clientAddr = {};
		int nAddrLen = sizeof(sockaddr_in);
		SOCKET _cSock = INVALID_SOCKET;
		_cSock = accept(_sock, (sockaddr*)& clientAddr, &nAddrLen);
		if (INVALID_SOCKET == _cSock)
		{
			printf("client connect error!\n");
		}
		else
		{
			printf("new client connect : IP = %s \n", inet_ntoa(clientAddr.sin_addr));
		}
		char msgBuf[] = "Hello , I'm server";
		send(_cSock, msgBuf, strlen(msgBuf) + 1, 0);
		char recvBuf[256] = {};
		int nlen = recv(_cSock, recvBuf, 256, 0);
		if (nlen > 0)
		{
			char msgBuf1[] = "127.0.0.1";
			printf("receive message: %s\n", recvBuf);
			send(_cSock, msgBuf1, strlen(msgBuf1) + 1, 0);
		}

	}
	closesocket(_sock);
	WSACleanup();
	return 0;
}