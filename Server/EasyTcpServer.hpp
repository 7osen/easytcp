#ifndef _EasyTcpServer_hpp_
#define _EasyTcpServer_hpp_
#define WIN32_LEAN_AND_MEAN
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#ifdef _WIN32
	#define FD_SETSIZE 1024
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
#include <vector>
#include "Message.hpp"
#include "TimeCount.hpp"

#pragma comment(lib, "ws2_32.lib")

class EasyTcpServer
{
	TimeCount _timeC;
	SOCKET _sock = INVALID_SOCKET;
	std::vector<SOCKET> c_Sock;
	int _recvCount = 0;
	int _Lastpos = 0;
#define MsgBufSize 409600
	char _MsgBuf[MsgBufSize] = {};//second
	char MsgBuf[MsgBufSize] = {};//first
public:
	EasyTcpServer()
	{

	}

	~EasyTcpServer()
	{

	}

	void init()
	{
		//windows socket 2.x 环境
#ifdef _WIN32
		WORD ver = MAKEWORD(2, 2);
		WSADATA dat;
		WSAStartup(ver, &dat);
#endif
		if (INVALID_SOCKET != _sock) Close();
		//建立socket
		_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (INVALID_SOCKET == _sock)
		{
			printf("build socket error...\n");
		}
		else
		{
			printf("build socket success...\n");
		}
		_timeC = TimeCount();
	}

	//Bind绑定端口
	void Bind(const char* ip, int port)
	{
		if (INVALID_SOCKET == _sock) init();
		sockaddr_in _sin = {};
		_sin.sin_family = AF_INET;
		_sin.sin_port = htons(port);
		if (ip)
		{
#ifdef _WIN32
			_sin.sin_addr.S_un.S_addr = inet_addr(ip);
#else
			_sin.sin_addr.s_addr = inet_addr(ip);
#endif
		}
		else
		{

#ifdef _WIN32
			_sin.sin_addr.S_un.S_addr = INADDR_ANY;
#else
			_sin.sin_addr.s_addr = INADDR_ANY;
#endif
		}
		int retbind = bind(_sock, (sockaddr*)& _sin, sizeof(_sin));
		if (SOCKET_ERROR == retbind)
		{
			printf("bind error!\n");
		}
		else
		{
			printf("bind success! ip = %s ...\n", inet_ntoa(_sin.sin_addr));
		}
	}

	//listen 监听端口
	void Listen()
	{
		int retlis = listen(_sock, 5);
		if (SOCKET_ERROR == retlis)
		{
			printf("listen error!\n");
		}
		else
		{
			printf("listen success!\n");
		}
	}

	void Run()
	{

		if (INVALID_SOCKET == _sock) init();
		while (isRun())
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
#ifdef _WIN32
				_cSock = accept(_sock, (sockaddr*)& clientAddr, &nAddrLen);
#else
				_cSock = accept(_sock, (sockaddr*)& clientAddr, (socklen_t*)& nAddrLen);
#endif
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
#ifdef _WIN32
			for (int i = 0; i < fRead.fd_count; i++)
			{
				if (-1 == Accept(fRead.fd_array[i]))
				{
					printf("client exit : Socket = %d ...\n", (int)fRead.fd_array[i]);
					auto iter = std::find(c_Sock.begin(), c_Sock.end(), fRead.fd_array[i]);
					if (iter != c_Sock.end())
					{
						c_Sock.erase(iter);
					}
				}
			}
#else
			for (auto i : c_Sock)
			{
				if (-1 == Accept(i))
				{
					printf("client exit : Socket = %d ...\n", (int)i);
					for (auto iter = c_Sock.begin(); iter != c_Sock.end(); iter++)
					{
						if (i == *iter) c_Sock.erase(iter);
						break;
					}
				}
			}
#endif
		}
	}

	bool isRun()
	{
		return _sock != INVALID_SOCKET;
	}

	int Accept(SOCKET cSock)
	{
		
		int nlen = Recieve(cSock);
		if (nlen > 0)
		{
			//if (!strcmp(recvBuf, "")) return 0;
			// char msgBuf1[] = "send message success!";
			
			// Send(cSock, msgBuf1);
			return 0;
		}
		else return -1;
	}

	int Recieve(SOCKET cSock)
	{
		char recvmsg[256] = {};
		int nlen = recv(cSock, MsgBuf, MsgBufSize, 0);
		if (nlen > 0)
		{
			_recvCount++;
			double t = _timeC.getSecond();
			if (t >= 1.0)
			{
				printf("time <%lf>, <SOCKET = %d>, Client = <%d>, recieve  = <%d> message ...\n", t, _sock, c_Sock.size(), _recvCount);
				_recvCount = 0;
				_timeC.Update();
			}
			memcpy(_MsgBuf + _Lastpos, MsgBuf, nlen);
			_Lastpos += nlen;
			while (_Lastpos >= sizeof(DataHeader))
			{
				DataHeader* header = (DataHeader*)_MsgBuf;
				if (_Lastpos >= header->HeaderLength)
				{
					memcpy(recvmsg, _MsgBuf + sizeof(DataHeader), header->DataLength);
					//printf("receive message from client <SOCKET = %d>: %s\n", cSock, recvmsg);
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

	void Send(SOCKET cSock, char* msg)
	{
		DataBody data = DataBody(msg);
		int res = send(cSock, (char*)& data, sizeof(data), 0);
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

private:

};

#endif