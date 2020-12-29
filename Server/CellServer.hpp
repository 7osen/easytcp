#ifndef  _CELL_SERVER_HPP_
#define _CELL_SERVER_HPP_

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
			printf("%d\n", (int)_time);
			return true;
		}
		return false;
	}
private:

};

class CellServer :public SendAndRecieveMessage
{
private:

	std::map<SOCKET, ClientInServer*> c_Sock_map;

	std::vector<ClientInServer*> c_Sock;
	std::vector<ClientInServer*> c_SockBuf;

	fd_set _fRead;
	std::mutex _m;
	TimeCount _oldTime;
	int _ID;
	SOCKET _sock = INVALID_SOCKET;
	int _ClientCount = 0;
	std::thread* _thread;

public:
	CellServer(int id, SOCKET sock = INVALID_SOCKET)
	{
		_ID = id;
		_sock = sock;

	}

	virtual ~CellServer()
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
					MaxSocket = std::max(MaxSocket, Sock->_sock);
#endif
					memcpy(&_fRead, &fRead, sizeof(fd_set));
				}
			else
			{
				memcpy(&fRead, &_fRead, sizeof(fd_set));
			}


			SOCKET_CHANGE = false;
			timeval Out_time;
			Out_time.tv_sec = 0;
			Out_time.tv_usec = 100000;
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
					ClientInServer* cis = *it;

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
	bool CheckHeart()
	{
		time_t dt = _oldTime.getMillSec();
		bool SOCKET_CHANGE = false;
		for (auto it = c_Sock.begin(); it != c_Sock.end();)
		{
			auto i = *it;
			if (i->CheckHeart(dt))
			{
				printf("client exit : Socket = %d ...\n", (int)i->_sock);
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

		int ret = Recieve(cSock);
		if (ret != -1)
		{
			c_Sock_map[cSock]->TimeReset();
			if (ret == CMD::HEART)
			{
				SendHeart(cSock);
			}
			return 0;
		}
		else return -1;
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
#endif