#ifndef  _Message_hpp_
#define _Message_hpp_
#include <string>
#include "TimeCount.hpp"
#ifdef __WIN32
	#include <Windows.h>
	#include <WinSock2.h>
#endif

enum CMD
{
	CMDERROR,
	MESSAGE,
	HEART
};

using std::string;

struct DataHeader
{
    unsigned short DataLength;
	unsigned short HeaderLength;
	CMD _cmd;
	DataHeader(int length,CMD cmd)
	{
		HeaderLength = sizeof(DataHeader) + length;
		DataLength = length;
		_cmd = cmd;
	}
};


struct DataBody :public DataHeader
{
	char data[10];
	DataBody(char* msg, CMD cmd = CMD::MESSAGE)
		:DataHeader(sizeof(DataBody) - sizeof(DataHeader),cmd)
	{
		strcpy(data, msg);
	}
};

struct HeartBody :public DataHeader
{
	HeartBody(CMD cmd = CMD::HEART)
		:DataHeader(sizeof(HeartBody) - sizeof(DataHeader), cmd)
	{

	}
};

class SendAndRecieveMessage
{
protected:


	const static int  MsgBufSize = 4096;
	char _MsgBuf[MsgBufSize] = {};//second
	char MsgBuf[MsgBufSize] = {};//first
	TimeCount _timeC;
	int _recvCount = 0;
	int _Lastpos = 0;
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

	virtual void Send(SOCKET cSock, char* msg)
	{
		DataBody data = DataBody(msg);
		int res = send(cSock, (char*) &data, sizeof(data), 0);
	}

	virtual void SendHeart(SOCKET cSock)
	{
		HeartBody data = HeartBody();
		int res = send(cSock, (char*) &data, sizeof(data), 0);
	}
};

#endif // ! _Message_hpp_
