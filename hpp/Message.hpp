#ifndef  _Message_hpp_
#define _Message_hpp_
#include <string>
#include "TimeCount.hpp"
#include <Windows.h>
#include <WinSock2.h>

using std::string;

struct DataHeader
{
	int DataLength;
	int HeaderLength;
	DataHeader(int length)
	{
		HeaderLength = sizeof(DataHeader) + length;
		DataLength = length;
	}
	virtual ~DataHeader(){}
};
struct DataBody :DataHeader
{
	char data[10];
	DataBody(char* msg)
		:DataHeader(12)
	{
		strcpy(data, msg);
	}
	virtual ~DataBody(){}
};
class SendAndRecieveMessage
{
protected:
	TimeCount _timeC;
	int _recvCount = 0;
	int _Lastpos = 0;

	const static int  MsgBufSize = 4096;
	char _MsgBuf[MsgBufSize] = {};//second
	char MsgBuf[MsgBufSize] = {};//first
public:
	SendAndRecieveMessage()
	{
		_timeC.Update();
	}
	virtual ~SendAndRecieveMessage()
	{

	}

	virtual int Recieve(SOCKET cSock)
	{
		char recvmsg[256] = {};
		int nlen = recv(cSock, MsgBuf, MsgBufSize, 0);
		if (nlen > 0)
		{

			double t = _timeC.getSecond();
			if (t >= 1.0)
			{
				printf("time <%lf>, <SOCKET = %d>,  recieve  = <%d> message ...\n", t, cSock, _recvCount);
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

	virtual int Recieve(SOCKET cSock,char* recvmsg)
	{

		int nlen = recv(cSock, MsgBuf, MsgBufSize, 0);
		if (nlen > 0)
		{

			double t = _timeC.getSecond();
			if (t >= 1.0)
			{
				printf("time <%lf>, <SOCKET = %d>,  recieve  = <%d> message ...\n", t, cSock, _recvCount);
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
					printf("receive message from <SOCKET = %d>: %s\n", cSock, recvmsg);
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

};

#endif // ! _Message_hpp_
