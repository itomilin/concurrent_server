#include "pch.h"
#pragma warning ( disable: 4996 )

#include "timer.h"

DWORD WINAPI EchoServer( LPVOID& item )
{
    // ��������� ���������� ������.
    auto async = std::async( std::launch::async, start_timer_async, &item );
    printf( "RUN_ECHO_SERVER\n" );

    // �������� ������� �� ������.
    auto client = (Contact*)&item;

    // ������ �����, ��� ������ ������� � ��������� ������������.
    client->SetST( Contact::WORK, "" );

    char msg[256] {};
    memset( msg, 0x00, sizeof( msg ) );

    MSG    msg_peek{};
    // ���� ��������� �� �������, ������� ����� �������� ��� �������.
    while ( true )
    {
        // ��������� ������������ ��������� ����� �� ��������� ������� �������.
        if ( PeekMessage( &msg_peek, NULL, WM_QUIT, WM_QUIT, PM_REMOVE ) == TRUE )
            break;

        if ( recv( client->clientSock, msg, sizeof( msg ), NULL ) != SOCKET_ERROR )
        {
            if ( std::strlen( msg ) == 0 )
            {
                // �������� ������.
                CancelWaitableTimer( client->htimer );
                // ������ �������, ��� ������ ������� ��������.
                client->SetST( Contact::FINISH, msg );
                // ������� �� �����.
                break;
            }

            // ���������� ����������� ��������� ������� �������.
            send( client->clientSock, msg, sizeof( msg ), NULL );
        }
        Sleep( 10 );
    }

    printf( "EXIT_ECHO_SERVER\n" );
    return 0u;
}

DWORD WINAPI TimeServer( LPVOID& item )
{
    // ��������� ���������� ������.
    auto async = std::async( std::launch::async, start_timer_async, &item );

    printf( "RUN_TIME_SERVER\n" );
    // �������� ������� �� ������.
    auto client = (Contact*)&item;
    // ������ �����, ��� ������ ������� � ��������� ������������.
    client->SetST( Contact::WORK, "" );

    char msg[256]{};
    memset( msg, 0x00, sizeof( msg ) );

    time_t rawtime;
    struct tm* timeinfo;
    char buffer[80];
 
    MSG    msg_peek{};
    // ���� ��������� �� �������, ������� ����� �������� ��� �������.
    while ( true )
    {
        // ��������� ������������ ��������� ����� �� ��������� ������� �������.
        if ( PeekMessage( &msg_peek, NULL, WM_QUIT, WM_QUIT, PM_REMOVE ) == TRUE )
            break;

        if ( recv( client->clientSock, msg, sizeof( msg ), NULL ) != SOCKET_ERROR )
        {
            if ( std::strcmp( "time", msg ) != 0 )
            {
                // �������� ������.
                CancelWaitableTimer( client->htimer );
                // ������ �������, ��� ������ ������� ��������.
                client->SetST( Contact::FINISH, msg );
                break;
            }

            time( &rawtime );
            timeinfo = localtime( &rawtime );
            strftime( buffer, 80, "%d.%m.%Y/%H:%M:%S", timeinfo );
            // ���������� ����������� ��������� ������� �������.
            send( client->clientSock, buffer, sizeof( buffer ), NULL );
        }
        Sleep( 10 );
    }
    
    printf( "EXIT_TIME_SERVER\n" );
    return 0u;
}

DWORD WINAPI RandServer( LPVOID& item )
{
    srand( static_cast<uint32_t>( time( NULL ) ) );
    // ��������� ���������� ������.
    auto async = std::async( std::launch::async, start_timer_async, &item );

    printf( "RUN_RAND_SERVER\n" );
    // �������� ������� �� ������.
    auto client = (Contact*)&item;
    // ������ �����, ��� ������ ������� � ��������� ������������.
    client->SetST( Contact::WORK, "" );

    char msg[256] {};
    memset( msg, 0x00, sizeof( msg ) );

    MSG    msg_peek{};
    // ���� ��������� �� �������, ������� ����� �������� ��� �������.
    while ( true )
    {
        // ��������� ������������ ��������� ����� �� ��������� ������� �������.
        if ( PeekMessage( &msg_peek, NULL, WM_QUIT, WM_QUIT, PM_REMOVE ) == TRUE )
            break;

        if ( recv( client->clientSock, msg, sizeof( msg ), NULL ) != SOCKET_ERROR )
        {
            if ( std::strcmp( "rand", msg ) != 0 )
            {
                // �������� ������.
                CancelWaitableTimer( client->htimer );
                // ������ �������, ��� ������ ������� ��������.
                client->SetST( Contact::FINISH, msg );
                break;
            }

            uint32_t buf = rand() << 16;
            char num_char[10];
            // �������� buf � num_char.
            std::sprintf( num_char, "%i", buf );
            // ���������� ����������� ��������� ������� �������.
            send( client->clientSock, num_char, sizeof( num_char), NULL );
        }
        Sleep( 10 );
    }

    printf( "EXIT_RAND_SERVER\n" );
    return 0u;
}
