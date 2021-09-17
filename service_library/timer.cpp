#include "pch.h"
#include "timer.h"

// ����� ������ ��������� � ���������� ���������, ����������� ���� callback.
VOID CALLBACK timer_finish_callback( LPVOID lpArgToCompletionRoutine,
                                     DWORD  dwTimerLowValue,
                                     DWORD  dwTimerHighValue )
{
    (*(Contact*)lpArgToCompletionRoutine).SetST( Contact::TIMEOUT, "" );
    std::cout << "========Timer is signaled======" << std::endl;
    CancelWaitableTimer( ( *(Contact*)lpArgToCompletionRoutine ).htimer );

    /*
    * ����� �� ������������ ������� TerminateThread ��� ExitThread,
    * ���������� ���������, ������� ��������� ���� ������� �������.
    */
    PostThreadMessage( ( *(Contact*)lpArgToCompletionRoutine ).thread_id, WM_QUIT, 0, 0 );
    std::cout << "========EXIT_TIMER======" << std::endl;
}

// ����������� ����� ��� ������� �������.
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

    // ����������� ������ � ��������� �������.
    ( *(Contact*)client ).htimer = h_timer;

    // ��������� ������.
    if ( WaitForSingleObject( h_timer, INFINITE ) != WAIT_OBJECT_0 )
        printf( "WaitForSingleObject failed (%d)\n", GetLastError() );
    else
        printf( "Timer was signaled after 60 sec.\n" );

    CloseHandle( h_timer );
    printf( "...CLOSE_TIMER_HANDLE...\n" );
}
