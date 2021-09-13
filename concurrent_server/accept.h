#pragma once

#include <string>
#include <iostream>
#include <list>
#include <cstring>

#include <WinSock2.h>
#include <Windows.h>
#pragma comment( lib, "WS2_32.lib" )

#include "../defs/defs.h"

#include <chrono>
#include <thread>

using namespace std::chrono;

#define AS_SQUIRT 10

void fetchDataFromDB()
{
    std::cout << "thread idz: " << GetCurrentThreadId() << std::endl;
    
    while ( true )
    {
        std::cout << "thread idz: " << GetCurrentThreadId() << std::endl;
        //std::this_thread::sleep_for( seconds( 10 ) );
    }
    
    //std::cout << "Run timerz" << ( *(Contact*)client ).clientSock << std::endl;

    //std::cout << "AFTER SLEEPz" << ( *(Contact*)client ).clientSock << std::endl;
}

//VOID CALLBACK ASWtimer( LPVOID lpArgToCompletionRoutine, DWORD dwTimerLowValue, DWORD dwTimerHighValue )
VOID CALLBACK ASWtimer( ULONG_PTR client )
{
    std::cout << "thread id " << GetCurrentThreadId() << std::endl;
    std::cout << "Run timer" << ( *(Contact*)client ).clientSock << std::endl;
    
    std::cout << "AFTER SLEEP" << ( *(Contact*)client ).clientSock << std::endl;
    SleepEx( INFINITE, TRUE );
    //TerminateThread( (*(Contact*)client ).hthread, 0 );
    //while ( true )
    //{
    //    SleepEx( 100, TRUE );
    //}

  /*  if ( WaitForSingleObjectEx( (*(Contact*)client).htimer, INFINITE, TRUE ) != WAIT_OBJECT_0 )
        printf( "WaitForSingleObject failed (%d)\n", GetLastError() );
    else
        printf( "Timer was signaled. Need disconnect client.\n" );*/
}

VOID CALLBACK ASWtimer2( ULONG_PTR client )
{
    std::cout << "----" << GetCurrentThreadId() << std::endl;
    std::cout << "---" << ( *(Contact*)client ).clientSock << std::endl;
    //SleepEx( INFINITE, TRUE );
    /*  while ( true )
      {
          SleepEx( 100, TRUE );
      }*/
      /*  if ( WaitForSingleObjectEx( (*(Contact*)client).htimer, INFINITE, TRUE ) != WAIT_OBJECT_0 )
            printf( "WaitForSingleObject failed (%d)\n", GetLastError() );
        else
            printf( "Timer was signaled. Need disconnect client.\n" );*/
}

VOID CALLBACK ASFinishMessage( LPVOID lpArgToCompletionRoutine, DWORD dwTimerLowValue, DWORD dwTimerHighValue )
{
    //CancelWaitableTimer(  );
    std::cout << "CANCEL: " << std::endl;
    std::cout << "AFTER SLEEP22" << std::endl;

    //TerminateThread( (*(Contact*)client ).hthread, 0 );
}

HANDLE( *ts )( const char*, LPVOID& );

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

