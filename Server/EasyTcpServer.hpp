#ifndef _EasyTcpServer_hpp_
#define _EasyTcpServer_hpp_
#include "Cell.hpp"
#include "CellSelect.hpp"
#include "CellEpoll.hpp"
class EasyTcpServer
{
private:
	const static int _ServerThreadCount = 4;

	std::vector<SOCKET> c_Sock;
	std::vector<CellServer*> _Servers;

	SOCKET _sock = INVALID_SOCKET;

public:
	EasyTcpServer()
	{

	}

	virtual ~EasyTcpServer()
	{
		Close();
	}

	void init()
	{
		//windows socket 2.x ����
#ifdef _WIN32
		WORD ver = MAKEWORD(2, 2);
		WSADATA dat;
		WSAStartup(ver, &dat);
#endif
		if (INVALID_SOCKET != _sock) Close();
		//����socket
		_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (INVALID_SOCKET == _sock)
		{
			printf("build socket error...\n");
		}
		else
		{
			printf("build socket success...\n");
		}
	}

	//Bind�󶨶˿�
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
			printf("bind success! ip = %s ... port = %d \n", inet_ntoa(_sin.sin_addr), port);
		}
	}

	//listen �����˿�
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
	template<typename T>
	void Start()
	{
		for (int i = 0; i < _ServerThreadCount; i++)
		{
			CellServer* server = new T(i + 1, _sock);
			_Servers.push_back(server);
			server->Start();
		}
	}

	void Run()
	{
		while (isRun())
		{
			sockaddr_in clientAddr = {};
			int nAddrLen = sizeof(sockaddr_in);
			SOCKET _cSock = INVALID_SOCKET;
#ifdef _WIN32
			_cSock = accept(_sock, (sockaddr*)&clientAddr, &nAddrLen);
#else
			_cSock = accept(_sock, (sockaddr*)&clientAddr, (socklen_t*)&nAddrLen);
#endif
			if (INVALID_SOCKET == _cSock && isRun())
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
	}

	bool isRun()
	{
		return _sock != INVALID_SOCKET;
	}

	

	void Close()
	{
		if (_sock != INVALID_SOCKET)
		{
			_sock = INVALID_SOCKET;
#ifdef _WIN32
			closesocket(_sock);
			WSACleanup();
#else 
			close(_sock);
#endif
			for (auto server : _Servers)
			{
				server->Close();
				delete server;
			}

			printf("TcpServer exit...\n");
		}
	}


};

#endif