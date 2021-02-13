#ifndef  _CELL_HPP_
#define _CELL_HPP_

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

#include "Message.hpp"
#include "TimeCount.hpp"
#include "CELLSemaphore.hpp"

#include <stdio.h>
#include <vector>
#include <thread>
#include <mutex>
#include <functional>
#include <map>

#pragma comment(lib, "ws2_32.lib")
#endif // ! _CELL_HPP_
