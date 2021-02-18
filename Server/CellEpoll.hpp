#ifndef  _CELL_EPOLL_HPP_
#define _CELL_EPOLL_HPP_
#if __linux__
#include <sys/epoll.h>
#include <algorithm>
#include "CellServer.hpp"
#define EPOLL_SIZE 10240

class CellEpoll :public CellServer
{
private:
public:
	CellEpoll(int id, SOCKET sock = INVALID_SOCKET)
		:CellServer(id, sock)
	{
	}
    ~CellEpoll()
    {

    }

	int doctl(int epfd, int op, uint32_t edt, int fd)
	{
		epoll_event ev;
		ev.events = edt;
		ev.data.fd = fd;
		epoll_ctl(epfd, op, fd, &ev);
	}

	void Run()
	{
		_oldTime.Update();
		_timeC.Update();
		int epfd = epoll_create(EPOLL_SIZE);
		epoll_event events[EPOLL_SIZE] = {};
		while (isRun())
		{
			
			print();
			if (c_SockBuf.size())
			{
				std::lock_guard<std::mutex> lock(_m);  
				for (auto newClient : c_SockBuf)
				{
					c_Sock.emplace_back(newClient);
					doctl(epfd, EPOLL_CTL_ADD, EPOLLIN, (int)newClient->_sock);
				}
				c_SockBuf.clear();
			}
			if (!c_Sock.size())
			{
				_oldTime.Update();
				continue;

			}

			int n = epoll_wait(epfd,events,EPOLL_SIZE,1);
			for (int i = 0; i < n; i++)
			{
				// if (events[i].events & EPOLLERR) 
				// {
				// 	if (clientLeave(events[i].data.fd) == -1) printf("err\n");
				// }
				// else if (events[i].events & EPOLLHUP) 
				// {
				// 	if (clientLeave(events[i].data.fd) == -1) printf("err\n");
				// }
				// else 
				{
					if (events[i].events & EPOLLIN) RecvData(events[i].data.fd);
					if (events[i].events & EPOLLOUT) WriteData(events[i].data.fd);
				}
			}
			CheckHeart();
		}
		printf("CellServer <%d> Run end.. \n", _ID);
		_cs.Wakeup();
	}

	int clientLeave(int sock)
	{

		auto x = std::find(c_Sock.begin(),c_Sock.end(),c_Sock_map[sock]);
		if (x != c_Sock.end())
		{
			c_Sock.erase(x);
			_ClientCount--;
			//printf("%d exit\n",sock);
			c_Sock_map.erase(sock);
			close(sock);
			return 0;
		}
		else return -1;
	}

	int RecvData(int sock)
	{
		if (-1 == Accept(sock)) 
		{
			clientLeave(sock);
		}
	}
		
	int WriteData(int sock)
	{

	}
	


};
#endif
#endif