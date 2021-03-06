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
#include <functional>
#include <mutex>
#include "Memory.hpp"
#include "Message.hpp"
#include "CELLSemaphore.hpp"


#pragma comment(lib, "ws2_32.lib")
class EasyTcpClient:public CellSemaphore
{
private:
	static const int MAX_HEART_TIME = 5000;
	static const int SEND_BUF_SIZE = 4096;
	static const int SEND_BUF_TIME = 200;

	const static int  MsgBufSize = 4096;
	char _MsgBuf[MsgBufSize] = {};//second
	char MsgBuf[MsgBufSize] = {};//first

	char _sendBuf[SEND_BUF_SIZE] = {};
	std::thread* _thread;
	TimeCount _send_time;
	TimeCount _heart_time;
	TimeCount _timeC;

	std::mutex _heart_time_mutex;
	std::mutex _send_mutex;
	SOCKET _sock = INVALID_SOCKET;
	int _recvCount = 0;
	int _Lastpos = 0;
	int _sendBufLen = 0;
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
		//windows socket 2.x ����
#ifdef _WIN32
		WORD ver = MAKEWORD(2, 2);
		WSADATA dat;
		WSAStartup(ver, &dat);
#endif
		//����socket
		if (_sock != INVALID_SOCKET) Close();
		_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (SOCKET_ERROR == _sock)
		{
			printf("build socket error!\n");
		}
		else
		{
			printf("build socket = %d success!\n",_sock);
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
			Close();
			return -1;
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
			_sock = INVALID_SOCKET;
#ifdef _WIN32
			closesocket(_sock);
#endif
			Wait();
			printf("SOCKET = %d Client Close...\n", _sock);
		}
	}
	
	bool isRun()
	{
		return _sock != INVALID_SOCKET;
	}
    
	void Send(SOCKET csock,char* msg)
	{
		//SendAndRecieveMessage::Send(_sock, msg);
		if (_sendBufLen + sizeof(DataBody) >= SEND_BUF_SIZE) OntimeSend();

		_send_mutex.lock();
		DataBody data = DataBody(msg);
		memcpy(_sendBuf + _sendBufLen, (char*)& data, sizeof(DataBody));
		_sendBufLen += sizeof(DataBody);
		_send_mutex.unlock();

		_heart_time_mutex.lock();
		_heart_time.Update();
		_heart_time_mutex.unlock();
	}

	
	void sendNow(char* msg)
	{

		//SendAndRecieveMessage::Send(_sock, msg);

		DataBody data = DataBody(msg);
		memcpy(_sendBuf + _sendBufLen, (char*)&data, sizeof(DataBody));
		_sendBufLen += sizeof(DataBody);
		send(_sock, _sendBuf, _sendBufLen, 0);
		_sendBufLen = 0;
	}

	void Send(char* msg)
	{
		
		//SendAndRecieveMessage::Send(_sock, msg);
		if (_sendBufLen + sizeof(DataBody) >= SEND_BUF_SIZE) OntimeSend();

		_send_mutex.lock();

		DataBody data = DataBody(msg);
		memcpy(_sendBuf + _sendBufLen, (char*) &data, sizeof(DataBody));
		_sendBufLen += sizeof(DataBody);
		_send_mutex.unlock();

		_heart_time_mutex.lock();
		_heart_time.Update();
		_heart_time_mutex.unlock();
	}

	void SendHeart(SOCKET csock)
	{
		//SendAndRecieveMessage::Send(_sock, msg);
		if (_sendBufLen + sizeof(HeartBody) >= SEND_BUF_SIZE) OntimeSend();

		_send_mutex.lock();

		HeartBody data = HeartBody();
		memcpy(_sendBuf + _sendBufLen, (char*)& data, sizeof(HeartBody));
		_sendBufLen += sizeof(HeartBody);
		_send_mutex.unlock();

		_heart_time.Update();
	}

	void OntimeSend()
	{

		_send_mutex.lock();

		send(_sock, _sendBuf, _sendBufLen, 0);
		_sendBufLen = 0;
		_send_time.Update();
		
		_send_mutex.unlock();

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
						//		printf("receive message from client <SOCKET = %d>: %s\n", cSock, recvmsg);
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

	void Run()
	{
		if (INVALID_SOCKET == _sock) Init();
		_send_time.Update();
		_heart_time.Update();
		_timeC.Update();
		while (isRun())
		{
			if (_send_time.getMillSec() >= SEND_BUF_TIME)
			{
				OntimeSend();
			}
			if (_recvCount)
			{
				double t = _timeC.getSecond();
				if (t >= 1.0)
				{
					printf("Client = %d,time <%lf>, recieve  = <%d> message ...\n",  _sock, t, _recvCount);
					_recvCount = 0;
					_timeC.Update();
				}
			}
			fd_set fRead;

			FD_ZERO(&fRead);

			FD_SET(_sock, &fRead);

			CheckHeart();

			timeval Out_time;
			Out_time.tv_sec = 0;
			Out_time.tv_usec = 100000;
			int ret = select(_sock + 1, &fRead, nullptr, nullptr, &Out_time);
			if (ret == 0) continue;
			if (ret < 0) Close();
			if (FD_ISSET(_sock, &fRead))
			{
				if (-1 == Recieve(_sock))
				{
					printf("connect break...\n");
					Close();
				}	
			}
		}
		Wakeup();
	}



	bool CheckHeart()
	{
		_heart_time_mutex.lock();
		int last_time  = _heart_time.getMillSec();
		if (last_time > MAX_HEART_TIME)
		{
			SendHeart(_sock);
			_heart_time.Update();
		}
		_heart_time_mutex.unlock();
		return false;
	}

	void Start()
	{
		_thread = new std::thread(std::mem_fun(&EasyTcpClient::Run), this);
	}
	
};

#endif // !_EasyTcpClient_hpp_
