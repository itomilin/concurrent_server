#include "pch.h"
#include "timer.h"

// Когда таймер переходит в сигнальное положение, срабатывает этот колбек.
VOID CALLBACK timer_finish_callback( LPVOID lpArgToCompletionRoutine, DWORD dwTimerLowValue, DWORD dwTimerHighValue )
{
   ///*Contact client = */( *(Contact*)lpArgToCompletionRoutine ).TIMEOUT;
    (*(Contact*)lpArgToCompletionRoutine).SetST( Contact::TIMEOUT, "" );
    //client.
    std::cout << "========Timer is signaled======" << std::endl;
    //CancelWaitableTimer( parg.htimer );
    // Если таймер срабатывает, нужно завершить поток обслуживающего сервера здесь.
    // дескриптор потока передать в параметре
    //ExitThread( (DWORD)&lpArgToCompletionRoutine );
    std::cout << "========EXIT_TIMER======" << std::endl;
}

// Асинхронный метод для запуска таймера.
void start_timer_async( LPVOID client )
{
    HANDLE h_timer = NULL;
    LARGE_INTEGER liDueTime{};
    liDueTime.QuadPart = -60000000LL;

    // Create an unnamed waitable timer.
    h_timer = CreateWaitableTimer( NULL, TRUE, NULL );
    if ( h_timer == NULL )
    {
        printf( "CreateWaitableTimer failed (%d)\n", GetLastError() );
        //return;
    }

    printf( "Waiting for 60 seconds...\n" );

    // Set a timer to wait for 5 seconds.
    if ( !SetWaitableTimer( h_timer, &liDueTime, NULL, timer_finish_callback, client, FALSE ) )
    {
        printf( "SetWaitableTimer failed (%d)\n", GetLastError() );
    }

    // Привязываем таймер в структуру клиента.
    ( *(Contact*)client ).htimer = h_timer;
    //auto tt = *(Contact*)&client;
    //tt.htimer = h_timer;
    printf( "SetWaitableTimer failed (%i)\n", ( *(Contact*)client ).htimer );
    printf( "SetWaitableTimer failed2 (%i)\n", h_timer );
    //// Привязываем таймер в структуру клиента.
    //( (Contact*)data )->htimer = h_timer;
    //printf( "SetWaitableTimer failed (%d)\n", ( (Contact*)data )->htimer );

    // Запускаем таймер.
    if ( WaitForSingleObject( h_timer, INFINITE ) != WAIT_OBJECT_0 )
        printf( "WaitForSingleObject failed (%d)\n", GetLastError() );
    else
        printf( "Timer was signaled after 60 sec.\n" );

    CloseHandle( h_timer );

    printf( "...CLOSE_HANDLE...\n" );
}
