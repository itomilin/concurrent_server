#pragma once

#include <string>
#include <iostream>
#include <list>
#include <cstring>
//#include <future>
//#include <chrono>
#include <sstream>

#include <WinSock2.h>
#include <Windows.h>
#pragma comment( lib, "WS2_32.lib" )

#include "../defs/defs.h"

//using namespace std::chrono;

#define AS_SQUIRT 10

HANDLE my_event;

HANDLE( *ts )( const char*, LPVOID& );

// Структура для передачи информации между потоками.
struct AcceptData final
{
    SOCKET serverSocket;
    TalkersCommand cmd;
    int16_t port;
    int32_t squirt;
};

// Дескрипторы потоков.
HANDLE hAcceptServer;
HANDLE hDispatchServer;
HANDLE hGarbageCleaner;
HANDLE hConsolePipe;
HANDLE hResponseServer;

// Обработка ошибок WINsock2.
std::string errorHandler( const std::string& msg, const int32_t& retCode );
bool AcceptCycle( AcceptData& acceptData );

DWORD WINAPI acceptServer( LPVOID cmd );
DWORD WINAPI dispatchServer( LPVOID cmd );
DWORD WINAPI garbageCleaner( LPVOID cmd );
DWORD WINAPI consolePipe( LPVOID cmd );
DWORD WINAPI responseServer( LPVOID cmd );

// TODO: Вынести в поле класса.
SOCKET SERVER_SOCK{};

bool putAnswerToClient( const char* name,
    const sockaddr* to,
    const int32_t* lTo,
    const SOCKET& serverSock );
