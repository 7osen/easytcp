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
#include "Message.hpp"

#pragma comment(lib, "ws2_32.lib")
class EasyTcpClient
{
	SOCKET _sock = INVALID_SOCKET;
	int _Lastpos = 0;
	static const int MsgBufSize = 409600;
	char _MsgBuf[MsgBufSize] = "";//second
	char MsgBuf[MsgBufSize] = "";//first
public:
	EasyTcpClient()
	{
	}

	virtual ~EasyTcpClient()
	{
		Close();
	}

	void Init()
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
		if (_sock == INVALID_SOCKET) Init();
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
			printf("Client Close...\n");
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

	int Recieve(char* recvmsg)
	{
		int nlen = recv(_sock, MsgBuf, MsgBufSize, 0);
		if (nlen > 0)
		{
			memcpy(_MsgBuf + _Lastpos, MsgBuf, nlen);
			_Lastpos += nlen;
			while (_Lastpos >= sizeof(DataHeader))
			{
				DataHeader* header = (DataHeader*)_MsgBuf;
				if (_Lastpos >= header->HeaderLength)
				{
					memcpy(recvmsg, _MsgBuf + sizeof(DataHeader), header->DataLength);
					int leftnum = _Lastpos - header->HeaderLength;
					memcpy(_MsgBuf, _MsgBuf + header->HeaderLength, leftnum);
					_Lastpos = leftnum;
				}
				else break;
			}
			return nlen;
		}
		else
		{
			return -1;
		}
		return nlen;
	}

	void SendMessage(char * msg)
	{
		DataHeader header = DataHeader(strlen(msg)+1);
		int res = send(_sock, (char*)& header, sizeof(DataHeader), 0);
		res =send(_sock, msg, strlen(msg) + 1, 0);
	}

	void Run()
	{

		if (INVALID_SOCKET == _sock) Init();
		
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
			int ret = select(_sock + 1, &fRead, &fWrite, &fExp, NULL);
			if (ret < 0)
			{
				printf("select end...\n");
			}
			if (FD_ISSET(_sock, &fRead))
			{
				char recvBuf[256] = "";
				if (-1 == Recieve(recvBuf))
				{
					printf("connect break...");
					Close();
				}
				else if (strcmp(recvBuf, ""))
				{
					printf("%s \n", recvBuf);
				}
			}
		}
	}
private:
	
};
#endif // !_EasyTcpClient_hpp_
