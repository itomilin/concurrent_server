#include "pch.h"
#pragma warning ( disable: 4996 )

#include "timer.h"

DWORD WINAPI EchoServer( LPVOID& item )
{
    // Запускаем асинхронно таймер.
    printf( "=======BEFORE ASYNC=========" );
    auto async = std::async( std::launch::async, start_timer_async, &item );
    printf( "=======AFTER ASYNC=========" );
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

    // Ждем сообщения от клиента, которое затем отправим ему обратно.
    while ( true )
    {
        auto out = recv( client->clientSock, msg, sizeof( msg ), NULL );
        if ( out != SOCKET_ERROR )
        {
            if ( std::strlen( msg ) == 0 )
            {
                printf( "From serviceserver timer cancel %i", client->htimer );
                // Отменяем таймер.
                CancelWaitableTimer( client->htimer );
                // Ставим отметку, что клиент успешно обслужен.
                client->SetST( Contact::FINISH, msg );
                break;
            }
            //printf( "From client: %s\n", msg );
            // Отправляем прочитанное сообщение обратно клиенту.
            send( client->clientSock, msg, sizeof( msg ), NULL );
        }
    }

    printf( "EXIT_ECHO_SERVER\n" );
    ExitThread( (DWORD)&item );
}

DWORD WINAPI TimeServer( LPVOID& item )
{
    // Запускаем асинхронно таймер.
    printf( "=======BEFORE ASYNC=========" );
    auto async = std::async( std::launch::async, start_timer_async, &item );
    printf( "=======AFTER ASYNC=========" );

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
            if ( std::strcmp( "time", msg ) != 0 )
            {
                // Отменяем таймер.
                CancelWaitableTimer( client->htimer );
                // Ставим отметку, что клиент успешно обслужен.
                client->SetST( Contact::FINISH, msg );
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

DWORD WINAPI RandServer( LPVOID& item )
{
    // Запускаем асинхронно таймер.
    printf( "=======BEFORE ASYNC=========" );
    auto async = std::async( std::launch::async, start_timer_async, &item );
    printf( "=======AFTER ASYNC=========" );

    printf( "RUN_RAND_SERVER\n" );
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

    // Ждем сообщения от клиента, которое затем отправим ему обратно.
    while ( true )
    {
        //Sleep( 1 );
        auto out = recv( client->clientSock, msg, sizeof( msg ), NULL );
        if ( out != SOCKET_ERROR )
        {
            if ( std::strcmp( "rand", msg ) != 0 )
            {
                // Отменяем таймер.
                CancelWaitableTimer( client->htimer );
                // Ставим отметку, что клиент успешно обслужен.
                client->SetST( Contact::FINISH, msg );
                break;
            }
            srand( time( NULL ) );
            uint32_t buf = rand() << 16;
            char num_char[10];
            // Копируем buf в num_char.
            std::sprintf( num_char, "%i", buf );
            // Отправляем прочитанное сообщение обратно клиенту.
            send( client->clientSock, num_char, sizeof( num_char), NULL );
        }
    }

    printf( "EXIT_RAND_SERVER\n" );
    ExitThread( (DWORD)&item );
}
