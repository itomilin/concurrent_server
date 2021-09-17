#include "pch.h"
#include "timer.h"

// Когда таймер переходит в сигнальное положение, срабатывает этот callback.
VOID CALLBACK timer_finish_callback( LPVOID lpArgToCompletionRoutine,
                                     DWORD  dwTimerLowValue,
                                     DWORD  dwTimerHighValue )
{
    (*(Contact*)lpArgToCompletionRoutine).SetST( Contact::TIMEOUT, "" );
    std::cout << "========Timer is signaled======" << std::endl;
    CancelWaitableTimer( ( *(Contact*)lpArgToCompletionRoutine ).htimer );

    /*
    * Чтобы не использовать опасные TerminateThread или ExitThread,
    * отправляем сообщение, которое разрывает цикл общения клиента.
    */
    PostThreadMessage( ( *(Contact*)lpArgToCompletionRoutine ).thread_id, WM_QUIT, 0, 0 );
    std::cout << "========EXIT_TIMER======" << std::endl;
}

// Асинхронный метод для запуска таймера.
void start_timer_async( LPVOID client )
{
    HANDLE h_timer = NULL;
    LARGE_INTEGER liDueTime{};
    liDueTime.QuadPart = -600000000LL;

    // Create an unnamed waitable timer.
    h_timer = CreateWaitableTimer( NULL, TRUE, NULL );
    if ( h_timer == NULL )
        throw ( "CreateWaitableTimer failed (%d)" + GetLastError() );

    printf( "Waiting for 60 seconds...\n" );

    // Set a timer to wait for 5 seconds.
    if ( !SetWaitableTimer( h_timer, &liDueTime, NULL, timer_finish_callback, client, FALSE ) )
        throw ( "SetWaitableTimer failed (%d)" + GetLastError() );

    // Привязываем таймер в структуру клиента.
    ( *(Contact*)client ).htimer = h_timer;

    // Запускаем таймер.
    if ( WaitForSingleObject( h_timer, INFINITE ) != WAIT_OBJECT_0 )
        printf( "WaitForSingleObject failed (%d)\n", GetLastError() );
    else
        printf( "Timer was signaled after 60 sec.\n" );

    CloseHandle( h_timer );
    printf( "...CLOSE_TIMER_HANDLE...\n" );
}
