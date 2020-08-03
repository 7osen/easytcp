#define WIN32_LEAN_AND_MEAN
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <Windows.h>
#include <WinSock2.h>
#include <stdio.h>
#include <vector>

#pragma comment(lib, "ws2_32.lib")
std::vector<SOCKET> c_Sock;

int myaccept(SOCKET _cSock)
{
	char recvBuf[256] = {};
	int nlen = recv(_cSock, recvBuf, 256, 0);
	if (nlen > 0)
	{
		char msgBuf1[] = "send message success!";
		printf("receive message from client <SOCKET = %d>: %s\n",_cSock, recvBuf);
		send(_cSock, msgBuf1, strlen(msgBuf1) + 1, 0);
		return 0;
	}
	else return -1;
}
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
		fd_set fRead;
		fd_set fWrite;
		fd_set fExp;

		FD_ZERO(&fRead);
		FD_ZERO(&fWrite);
		FD_ZERO(&fExp);

		FD_SET(_sock, &fRead);
		FD_SET(_sock, &fWrite);
		FD_SET(_sock, &fExp);
		for (auto _csock : c_Sock)
		{
			FD_SET(_csock, &fRead);
		}
		int ret = select(_sock + 1, &fRead, &fWrite, &fExp, NULL);
		if (ret < 0)
		{
			printf("select end...\n");
			break;
		}
		if (FD_ISSET(_sock, &fRead))
		{
			FD_CLR(_sock, &fRead);
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
				
				printf("new client connect : Socket = %d, IP = %s ...\n", (int)_cSock, inet_ntoa(clientAddr.sin_addr));
				c_Sock.push_back(_cSock);
			}
		}
		for (int i = 0; i < fRead.fd_count; i++)
		{
			if (-1 == myaccept(fRead.fd_array[i]))
			{
				printf("client exit : Socket = %d ...\n", (int)fRead.fd_array[i]);
				auto iter = std::find(c_Sock.begin(), c_Sock.end(), fRead.fd_array[i]);
				if (iter != c_Sock.end())
				{
					c_Sock.erase(iter);
				}
			}
		}

	}
	closesocket(_sock);
	WSACleanup();
	return 0;
}