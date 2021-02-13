#ifndef  _CELL_SERVER_HPP_
#define _CELL_SERVER_HPP_

#include "Cell.hpp"

class ClientInServer
{
private:
	static const int MAX_HEART_TIME = 10000;
	int _time;
public:
	SOCKET _sock;
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

	bool CheckHeart(int dt)
	{
		_time += dt;
		if (_time > MAX_HEART_TIME)
		{
			return true;
		}
		return false;
	}
private:

};

class CellServer
{
protected:

	const static int  MsgBufSize = 4096;
	char _MsgBuf[MsgBufSize] = {};//second
	char MsgBuf[MsgBufSize] = {};//first
	CellSemaphore _cs;
	std::map<SOCKET, ClientInServer*> c_Sock_map;

	
	std::vector<ClientInServer*> c_Sock;
	std::vector<ClientInServer*> c_SockBuf;

	std::mutex _m;
	TimeCount _oldTime;
	TimeCount _timeC;
	SOCKET _sock = INVALID_SOCKET;
	std::thread* _thread;
	int _recvCount = 0;
	int _Lastpos = 0;
	int _ClientCount = 0;

	bool SOCKET_CHANGE = false;
public:
	int _ID;
	CellServer(int id, SOCKET sock = INVALID_SOCKET)
	{
		_ID = id;
		_sock = sock;

	}

	virtual ~CellServer()
	{
		Close();
	}

	void print()
	{
		double t = _timeC.getSecond();
		if (t >= 1.0)
		{
			printf("Server = %d,time <%lf>, <SOCKET = %d>, Client = <%d>, recieve  = <%d> message ...\n", _ID, t, _sock, (int)c_Sock.size(), _recvCount);
			_recvCount = 0;
			_timeC.Update();
		}
	}

	virtual void Run() = 0;

	bool CheckHeart()
	{
		int dt = _oldTime.getMillSec();
		bool SOCKET_CHANGE = false;
		for (auto it = c_Sock.begin(); it != c_Sock.end();)
		{
			auto i = *it;
			if (i->CheckHeart(dt))
			{
				//printf("client exit : Socket = %d ...\n", (int)i->_sock);
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

	int Recieve(SOCKET cSock)
	{
		char recvmsg[256] = {};
		int nlen = recv(cSock, MsgBuf, MsgBufSize, 0);
		int ret = -1;
		if (nlen > 0)
		{
			ret = CMD::MESSAGE;
			memcpy(_MsgBuf + _Lastpos, MsgBuf, nlen);
			_Lastpos += nlen;
			while (_Lastpos >= sizeof(DataHeader))
			{
				DataHeader* header = (DataHeader*)_MsgBuf;
				if (_Lastpos >= header->HeaderLength)
				{
					_recvCount++;
					if (header->_cmd == CMD::MESSAGE)
					{
						memcpy(recvmsg, _MsgBuf + sizeof(DataHeader), header->DataLength);
						//printf("receive message from client <SOCKET = %d>: %s\n", cSock, recvmsg);
					}
					else if (header->_cmd == CMD::HEART)
					{
						ret = CMD::HEART;
					}
					int leftnum = _Lastpos - header->HeaderLength;
					memcpy(_MsgBuf, _MsgBuf + header->HeaderLength, leftnum);
					_Lastpos = leftnum;
				}
				else break;
			}
			return ret;
		}
		else
		{
			return -1;
		}
		return ret;
	}

	void SendHeart(SOCKET cSock)
	{
		HeartBody data = HeartBody();
		int res = send(cSock, (char*)&data, sizeof(data), 0);
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
		if (_sock != INVALID_SOCKET)
		{
			printf("CellServer <%d> close begin..\n", _ID);
			_sock = INVALID_SOCKET;
			_cs.Wait();
			printf("CellServer <%d> close..\n",_ID);
		}
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