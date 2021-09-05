#include "pch.h"

#include "PrototypeService.h"

DWORD WINAPI EchoServer( LPVOID& item )
{
    // �������� ������� �� ������.
    auto client = (Contact*)&item;
    // ������ �����, ��� ������ ������� � ��������� ������������.
    client->SetST( Contact::WORK, "" );
    //client->srvname = "EchoServer";

    char msg[256] {};
    memset( msg, 0x00, sizeof( msg ) );

    // TODO. �������� ������ ������ recv ���������� �� socket_error < 0
    // �� �����=�� ������� ����� msg ��������� ������ � ������� recv
    // memset ������ ������� �� ����
    recv( client->clientSock, msg, sizeof( msg ), NULL );

    // ���� ��������� �� �������, ������� ����� �������� ��� �������.
    while ( true )
    {
        Sleep( 1 );
        auto out = recv( client->clientSock, msg, sizeof( msg ), NULL );
        if ( out != SOCKET_ERROR )
        {
            if ( std::strlen( msg ) == 0 ) {
                // ������ �������, ��� ������ ������� ��������.
                client->SetST( Contact::FINISH, msg );
                closesocket( client->clientSock );
                break;
            }
            printf( "From client: %s\n", msg );
            // ���������� ����������� ��������� ������� �������.
            send( client->clientSock, msg, sizeof( msg ), NULL );
        }
    }

    // ��������� ������������� �����.
    ExitThread( (DWORD)&item );
}

DWORD WINAPI TimeServer( LPVOID& item )
{
    ExitThread( (DWORD)&item );
}

DWORD WINAPI ServiceServer( LPVOID& item )
{
    ExitThread( (DWORD)&item );
}
