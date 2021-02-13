#ifndef  _CELL_SELECT_HPP_
#define _CELL_SELECT_HPP_

#include "CellServer.hpp"

class CellSelect :public CellServer
{
private:
	fd_set _fd;
public:
	CellSelect(int id, SOCKET sock = INVALID_SOCKET)
		:CellServer(id, sock)
	{
	}
	~CellSelect()
	{
	}
	void Run()
	{
		_oldTime.Update();
		FD_ZERO(&_fd);
		FD_SET(_sock, &_fd);
		SOCKET MaxSocket = _sock;
		_timeC.Update();
		while (isRun())
		{
			
			print();
			if (c_SockBuf.size())
			{
				_m.lock();
				for (auto newClient : c_SockBuf)
				{
					c_Sock.push_back(newClient);
					FD_SET(newClient->_sock, &_fd);
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

			fd_set fWrite;
			FD_ZERO(&fWrite);
			FD_SET(_sock, &fWrite);
			if (SOCKET_CHANGE)
			{
				for (auto Sock : c_Sock)
				{

					FD_SET(Sock->_sock, &fRead);
#ifdef _WIN32
					MaxSocket = max(MaxSocket, Sock->_sock);
#else
					MaxSocket = std::max(MaxSocket, Sock->_sock);
#endif
				}
				memcpy(&_fd, &fRead, sizeof(fd_set));
			}
			else
			{
				memcpy(&fRead, &_fd, sizeof(fd_set));
			}
			memcpy(&fWrite, &_fd, sizeof(fd_set));

			SOCKET_CHANGE = false;
			timeval Out_time;
			Out_time.tv_sec = 0;
			Out_time.tv_usec = 100000;
			int ret = select(MaxSocket + 1, &fRead, &fWrite, nullptr, &Out_time);
			SOCKET_CHANGE = CheckHeart();
			if (ret <= 0)
			{
				continue;
			}
			FD_CLR(_sock, &fRead);
			RecvData(fRead);
			WriteData(fWrite);
		}
		printf("CellServer <%d> Run end.. \n", _ID);
		_cs.Wakeup();
	}

	void WriteData(fd_set fWrite)
	{

	}

	void RecvData(fd_set fRead)
	{

#ifdef _WIN32
		for (int i = 0; i < (int)fRead.fd_count; i++)
		{
			if (-1 == Accept(fRead.fd_array[i]))
			{
				//printf("client exit : Socket = %d ...\n", fRead.fd_array[i]);
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
				//printf("client exit : Socket = %d ...\n", i);
				SOCKET_CHANGE = true;
				ClientInServer* cis = *it;

				it = c_Sock.erase(it);
				c_Sock_map.erase(i);
				delete cis;
			}
			else it++;
		}
#endif
	}

	


};
#endif