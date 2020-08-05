#ifndef _EasyTcpClient_hpp_
#define _EasyTcpClient_hpp_
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
class EasyTcpClient
{
	SOCKET _sock = INVALID_SOCKET;
public:
	EasyTcpClient()
	{
	}
	virtual ~EasyTcpClient()
	{
		Close();
	}
	void init()
	{
		//windows socket 2.x 环境
#ifdef _WIN32
		WORD ver = MAKEWORD(2, 2);
		WSADATA dat;
		WSAStartup(ver, &dat);
#endif
		//建立socket
		if (_sock != INVALID_SOCKET) Close();
		_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (SOCKET_ERROR == _sock)
		{
			printf("build socket error!\n");
		}
		else
		{
			printf("build socket success!\n");
		}
	}
	int Connect(char* IP, int port)
	{
		if (_sock == INVALID_SOCKET) init();
		sockaddr_in _sin = {};
		_sin.sin_family = AF_INET;
		_sin.sin_port = htons(port);
#ifdef _WIN32
		_sin.sin_addr.S_un.S_addr = inet_addr(IP);
#else 
		_sin.sin_addr.s_addr = inet_addr(IP);
#endif
		int retc = connect(_sock, (sockaddr*)& _sin, sizeof(sockaddr_in));
		if (SOCKET_ERROR == retc)
		{
			printf("connect error!\n");

		}
		else
		{
			printf("connect success! IP = %s ... \n", inet_ntoa(_sin.sin_addr));
		}
		return retc;
	}
	void Close()
	{
		if (_sock != INVALID_SOCKET)
		{

#ifdef _WIN32
			closesocket(_sock);
			WSACleanup();
#else 
			close(_sock);
#endif
		}
		_sock = INVALID_SOCKET;
	}
	
	bool isRun()
	{
	 //std::thread cmdt(cmdThread);
	 //cmdt.detach();
		return _sock != INVALID_SOCKET;
	}
	int recieve(char *recvmsg, int len,int flag)
	{
		int nlen = recv(_sock, recvmsg, len, flag);
		if (nlen > 0)
		{
			printf("receive message: %s\n", recvmsg);
		}
		return nlen;
	}
	void sendMessage(char * msgBuf)
	{
		send(_sock, msgBuf, strlen(msgBuf) + 1, 0);
	}
private:
	
};
#endif // !_EasyTcpClient_hpp_
