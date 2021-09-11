#include "pch.h"

#include "PrototypeService.h"
#include <ctime>
#include <sstream>

VOID CALLBACK ASWtimer( ULONG_PTR client )
{
    printf( "thread id: %i\n", GetCurrentThreadId() );
    printf( "thread id: %i\n", ( *(Contact*)client ).clientSock );
    char msg[256]{};
    memset( msg, 0x00, sizeof( msg ) );
    //SleepEx( INFINITE, TRUE );
    printf( "RUN_ECHO_SERER3333\n" );
    while ( true )
    {
        Sleep( 1 );
        auto out = recv( ( *(Contact*)client ).clientSock, msg, sizeof( msg ), NULL );
        if ( out != SOCKET_ERROR )
        {
            if ( std::strlen( msg ) == 0 ) {
                // Ставим отметку, что клиент успешно обслужен.
                ( *(Contact*)client ).SetST( Contact::FINISH, msg );
                closesocket( ( *(Contact*)client ).clientSock );
                break;
            }
            printf( "From client: %s\n", msg );
            // Отправляем прочитанное сообщение обратно клиенту.
            send( ( *(Contact*)client ).clientSock, msg, sizeof( msg ), NULL );
        }
    }

    //if ( WaitForSingleObjectEx( (*(Contact*)client).htimer, INFINITE, TRUE ) != WAIT_OBJECT_0 )
    //    printf( "WaitForSingleObject failed (%d)\n", GetLastError() );
    //else
    //    printf( "Timer was signaled. Need disconnect client.\n" );
}

VOID CALLBACK ASFinishMessage( LPVOID lpArgToCompletionRoutine, DWORD dwTimerLowValue, DWORD dwTimerHighValue )
{
    //CancelWaitableTimer(  );
    printf( "FINISH\n" );
    //TerminateThread( (*(Contact*)client ).hthread, 0 );
}

DWORD WINAPI EchoServer( LPVOID& item )
{
    printf( "RUN_ECHO_SERVER\n" );
    // Получаем клиента из списка.
    auto client = (Contact*)&item;
    // Ставим метку, что клиент перешел в состояние обслуживания.
    client->SetST( Contact::WORK, "" );
    //client->srvname = "EchoServer";

    char msg[256] {};
    memset( msg, 0x00, sizeof( msg ) );

    // TODO. Выяснить почему первый recv возвращает не socket_error < 0
    // По какой=то причине буфер msg очищается только в функции recv
    // memset такого эффекта не дает
    recv( client->clientSock, msg, sizeof( msg ), NULL );


    //client->htimer = CreateWaitableTimer( NULL, FALSE, L"timer1" );
    //if ( NULL == client->htimer )
    //    printf( "CreateWaitableTimer failed (%d)\n", GetLastError() );

    //LARGE_INTEGER fTime;
    //fTime.QuadPart = -60000000LL;
    //if ( !SetWaitableTimer( client->htimer, &fTime, NULL,
    //    ASFinishMessage, (LPVOID*)&item, FALSE) )
    //    printf( "SetWaitableTimer failed (%d)\n", GetLastError() );

    //QueueUserAPC( ASWtimer, client->hthread, (ULONG_PTR)&item );

    
    //if ( WaitForSingleObjectEx( ( *(Contact*)client ).htimer, INFINITE, TRUE ) != WAIT_OBJECT_0 )
    //    printf( "WaitForSingleObject failed (%d)\n", GetLastError() );
    //else
    //    printf( "Timer was signaled. Need disconnect client.\n" );

    //printf( "---RUN_ECHO_SERER2---\n" );
    // Ждем сообщения от клиента, которое затем отправим ему обратно.
    while ( true )
    {
        //Sleep( 1 );
        auto out = recv( client->clientSock, msg, sizeof( msg ), NULL );
        if ( out != SOCKET_ERROR )
        {
            if ( std::strlen( msg ) == 0 ) {
                // Ставим отметку, что клиент успешно обслужен.
                client->SetST( Contact::FINISH, msg );
                closesocket( client->clientSock );
                break;
            }
            //printf( "From client: %s\n", msg );
            // Отправляем прочитанное сообщение обратно клиенту.
            send( client->clientSock, msg, sizeof( msg ), NULL );
        }
    }

    CancelWaitableTimer( client->htimer );
    // Завершаем обслуживающий поток.
    ExitThread( (DWORD)&item );
}

DWORD WINAPI TimeServer( LPVOID& item )
{
    printf( "RUN_TIME_SERVER\n" );
    // Получаем клиента из списка.
    auto client = (Contact*)&item;
    // Ставим метку, что клиент перешел в состояние обслуживания.
    client->SetST( Contact::WORK, "" );
    //client->srvname = "EchoServer";

    char msg[256]{};
    memset( msg, 0x00, sizeof( msg ) );

    // TODO. Выяснить почему первый recv возвращает не socket_error < 0
    // По какой=то причине буфер msg очищается только в функции recv
    // memset такого эффекта не дает
    recv( client->clientSock, msg, sizeof( msg ), NULL );

    time_t rawtime;
    struct tm* timeinfo;
    char buffer[80];
 
    // Ждем сообщения от клиента, которое затем отправим ему обратно.
    while ( true )
    {
        //Sleep( 1 );
        auto out = recv( client->clientSock, msg, sizeof( msg ), NULL );
        if ( out != SOCKET_ERROR )
        {
            if ( std::strcmp( "time", msg ) != 0 ) {
                // Ставим отметку, что клиент успешно обслужен.
                client->SetST( Contact::FINISH, msg );
                closesocket( client->clientSock );
                printf( "CLOSE_RUN_TIME_SERVER\n" );
                break;
            }

            time( &rawtime );
            timeinfo = localtime( &rawtime );
            strftime( buffer, 80, "%d.%m.%Y/%H:%M:%S", timeinfo );
            // Отправляем прочитанное сообщение обратно клиенту.
            send( client->clientSock, buffer, sizeof( buffer ), NULL );
        }
    }
    printf( "EXIT_TIME_SERVER\n" );
    ExitThread( (DWORD)&item );
}

DWORD WINAPI ServiceServer( LPVOID& item )
{
    printf( "RUN_RAND_SERVER\n" );
    // Получаем клиента из списка.
    auto client = (Contact*)&item;
    // Ставим метку, что клиент перешел в состояние обслуживания.
    client->SetST( Contact::WORK, "" );
    //client->srvname = "EchoServer";

    char msg[256]{};
    memset( msg, 0x00, sizeof( msg ) );

    // TODO. Выяснить почему первый recv возвращает не socket_error < 0
    // По какой=то причине буфер msg очищается только в функции recv
    // memset такого эффекта не дает
    recv( client->clientSock, msg, sizeof( msg ), NULL );

    // Ждем сообщения от клиента, которое затем отправим ему обратно.
    while ( true )
    {
        //Sleep( 1 );
        auto out = recv( client->clientSock, msg, sizeof( msg ), NULL );
        if ( out != SOCKET_ERROR )
        {
            if ( std::strcmp( "rand", msg ) != 0 ) {
                // Ставим отметку, что клиент успешно обслужен.
                client->SetST( Contact::FINISH, msg );
                closesocket( client->clientSock );
                printf( "CLOSE_RAND_SERVER\n" );
                break;
            }
            srand( time( NULL ) );
            uint32_t buf = rand() << 16;
            char num_char[10];
            // Копируем buf в num_char.
            std::sprintf( num_char, "%d", buf );
            printf( "randnumv: %s\n", num_char );
            // Отправляем прочитанное сообщение обратно клиенту.
            send( client->clientSock, num_char, sizeof( num_char), NULL );
        }
    }
    printf( "EXIT_RAND_SERVER\n" );
    ExitThread( (DWORD)&item );
}
