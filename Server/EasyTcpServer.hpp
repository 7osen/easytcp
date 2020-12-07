#ifndef _EasyTcpServer_hpp_
#define _EasyTcpServer_hpp_
#define WIN32_LEAN_AND_MEAN
#define _WINSOCK_DEPRECATED_NO_WARNINGS


#ifdef _WIN32
#define FD_SETSIZE 4096
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

#include "Memory.hpp"
#include "Message.hpp"
#include "TimeCount.hpp"

#include <stdio.h>
#include <vector>
#include <thread>
#include <mutex>
#include <functional>
#include <map>

#pragma comment(lib, "ws2_32.lib")

class ClientInServer
{
public:
	static const int MAX_HEART_TIME = 10000;
	SOCKET _sock;
	time_t _time;
	ClientInServer(SOCKET sock) :_sock(sock)
	{
		TimeReset();
	}
	~ClientInServer()
	{
	}

	void TimeReset()
	{
		_time = 0;
	}

	bool CheckHeart(time_t dt)
	{
		_time += dt;
		if (_time > MAX_HEART_TIME)
		{
			printf("%d\n", _time);
			return true;
		}
		return false;
	}
private:

};

class CellServer :public SendAndRecieveMessage
{
private:
	int _ID;

	SOCKET _sock = INVALID_SOCKET;

	int _ClientCount = 0;

	std::vector<ClientInServer*> c_Sock;
	std::vector<ClientInServer*> c_SockBuf;
	std::thread* _thread;
	std::mutex _m;

	std::map<SOCKET, ClientInServer*> c_Sock_map;

	fd_set _fRead;
public:
	CellServer(int id, SOCKET sock = INVALID_SOCKET)
	{
		_ID = id;
		_sock = sock;

	}

	~CellServer()
	{

	}

	void Run()
	{
		_oldTime.Update();
		bool SOCKET_CHANGE = false;
		FD_ZERO(&_fRead);
		FD_SET(_sock, &_fRead);
		SOCKET MaxSocket = _sock;
		while (isRun())
		{
			double t = _timeC.getSecond();
			if (t >= 1.0)
			{
				printf("Server = %d,time <%lf>, <SOCKET = %d>, Client = <%d>, recieve  = <%d> message ...\n", _ID, t, _sock, (int)c_Sock.size(), _recvCount);
				_recvCount = 0;
				_timeC.Update();
			}

			if (c_SockBuf.size())
			{
				_m.lock();
				for (auto newClient : c_SockBuf)
				{
					c_Sock.push_back(newClient);
					FD_SET(newClient->_sock, &_fRead);
					if (newClient->_sock > MaxSocket) MaxSocket = newClient->_sock;
				}
				c_SockBuf.clear();
				_m.unlock();
			}
			if (!c_Sock.size())
			{
				_oldTime.Update();
				continue;
			    
			}

			fd_set fRead;
			FD_ZERO(&fRead);
			FD_SET(_sock, &fRead);
			if (SOCKET_CHANGE)
				for (auto Sock : c_Sock)
				{

					FD_SET(Sock->_sock, &fRead);
#ifdef _WIN32
					MaxSocket = max(MaxSocket, Sock->_sock);
#else
					MaxSocket = std::max(MaxSocket, Sock);
#endif
					memcpy(&_fRead, &fRead, sizeof(fd_set));
				}
			else
			{
				memcpy(&fRead, &_fRead, sizeof(fd_set));
			}


			SOCKET_CHANGE = false;
			timeval Out_time;
			Out_time.tv_sec = 1;
			Out_time.tv_usec = 0;
			int ret = select(MaxSocket + 1, &fRead, nullptr, nullptr, &Out_time);
			if (ret <= 0)
			{
				continue;
			}
			FD_CLR(_sock, &fRead);

#ifdef _WIN32
			for (int i = 0; i < (int)fRead.fd_count; i++)
			{
				if (-1 == Accept(fRead.fd_array[i]))
				{
					printf("client exit : Socket = %d ...\n", fRead.fd_array[i]);
					auto client = c_Sock_map[fRead.fd_array[i]];
					auto iter = std::find(c_Sock.begin(), c_Sock.end(), client);
					if (iter != c_Sock.end())
					{
						c_Sock.erase(iter);
						c_Sock_map.erase(fRead.fd_array[i]);
						delete client;
						SOCKET_CHANGE = true;
					}
				}
				
			}
			
			


#else
			for (auto it = c_Sock.begin(); it != c_Sock.end();)
			{
				auto i = (*it)->_sock;
				if (FD_ISSET(i, &fRead) && -1 == Accept(i))
				{
					printf("client exit : Socket = %d ...\n", i);
					SOCKET_CHANGE = true;
					ClientInServer cis = *it;

					it = c_Sock.erase(it);
					c_Sock_map.erase(i);
					delete cis;
				}
				else it++;
			}
#endif
			SOCKET_CHANGE = CheckHeart();
		}
	}
	TimeCount _oldTime;
	bool CheckHeart()
	{
		time_t dt = _oldTime.getMillSec();
		bool SOCKET_CHANGE = false;
		for (auto it = c_Sock.begin(); it != c_Sock.end();)
		{
			auto i = *it;
			if (i->CheckHeart(dt))
			{
				printf("client exit : Socket = %d ...\n", i->_sock);
				SOCKET_CHANGE = true;
				it = c_Sock.erase(it);
				c_Sock_map.erase(i->_sock);
				delete i;
			}
			else it++;
		}
		_oldTime.Update();
		return SOCKET_CHANGE;
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
			c_Sock_map[cSock]->TimeReset();
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

			
			memcpy(_MsgBuf + _Lastpos, MsgBuf, nlen);
			_Lastpos += nlen;
			while (_Lastpos >= sizeof(DataHeader))
			{
				DataHeader* header = (DataHeader*)_MsgBuf;
				if (_Lastpos >= header->HeaderLength)
				{
					_recvCount++;
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

	void Close()
	{
		_sock = INVALID_SOCKET;
	}
	int getCount()
	{
		return _ClientCount;
	}

	void addClient(SOCKET csock)
	{
		_m.lock();
		_ClientCount++;
		c_Sock_map[csock] = new ClientInServer(csock);
		c_SockBuf.push_back(c_Sock_map[csock]);
		_m.unlock();
	}

	void Start()
	{
		_thread = new std::thread(std::mem_fun(&CellServer::Run), this);
	}


};


class EasyTcpServer : public SendAndRecieveMessage
{
private:
	SOCKET _sock = INVALID_SOCKET;

	const static int _ServerThreadCount = 4;
	std::vector<CellServer*> _Servers;

	std::vector<SOCKET> c_Sock;
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

	void Start()
	{
		for (int i = 0; i < _ServerThreadCount; i++)
		{
			CellServer* server = new CellServer(i + 1, _sock);
			server->Start();
			_Servers.push_back(server);
		}
	}

	void Run()
	{
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
			//printf("new client connect : Socket = %d, IP = %s ...\n", (int)_cSock, inet_ntoa(clientAddr.sin_addr));
			CellServer* minServer = _Servers[0];
			for (auto server : _Servers)
			{
				if (server->getCount() < minServer->getCount())
				{
					minServer = server;
				}
			}
			minServer->addClient(_cSock);
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

			double t = _timeC.getSecond();
			if (t >= 1.0)
			{
				printf("time <%lf>, <SOCKET = %d>, Client = <%d>, recieve  = <%d> message ...\n", t, _sock, (int)c_Sock.size(), _recvCount);
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
					_recvCount++;
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


};

#endif