#pragma once

#include <string>
#include <iostream>
#include <list>
#include <cstring>

#include <WinSock2.h>
#include <Windows.h>
#pragma comment( lib, "WS2_32.lib" )

#include "../defs/defs.h"

#define AS_SQUIRT 10

VOID CALLBACK ASWtimer( LPVOID lpArgToCompletionRoutine, DWORD dwTimerLowValue, DWORD dwTimerHighValue )
{
    std::cout << "CALL ASWTIMER" << std::endl;
}

VOID CALLBACK ASFinishMessage( LPVOID lpArgToCompletionRoutine, DWORD dwTimerLowValue, DWORD dwTimerHighValue )
{
    //CancelWaitableTimer(  );
    std::cout << "CALL ASFINISHMEESAGE" << std::endl;
}

HANDLE( *ts )( const char*, LPVOID& );

// Команды для сервера.
enum TalkersCommand
{
    START,
    STOP,
    EXIT,
    STATISTICS,
    WAIT,
    SHUTDOWN,
    GETCOMMAND
};

struct AcceptData final
{
    SOCKET serverSocket;
    TalkersCommand cmd;
    int16_t port;
    int32_t squirt;
};

HANDLE hAcceptServer;
HANDLE hDispatchServer;
HANDLE hGarbageCleaner;
HANDLE hConsolePipe;

// Обработка ошибок WINsock2.
std::string errorHandler( const std::string& msg, const int32_t& retCode );
bool AcceptCycle( AcceptData& acceptData );

DWORD WINAPI acceptServer( LPVOID cmd );
DWORD WINAPI dispatchServer( LPVOID cmd );
DWORD WINAPI garbageCleaner( LPVOID cmd );
DWORD WINAPI consolePipe( LPVOID cmd );

