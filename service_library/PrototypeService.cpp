#include "pch.h"

#include "PrototypeService.h"

DWORD WINAPI EchoServer( LPVOID& item )
{
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
        Sleep( 1 );
        auto out = recv( client->clientSock, msg, sizeof( msg ), NULL );
        if ( out != SOCKET_ERROR )
        {
            if ( std::strlen( msg ) == 0 ) {
                // Ставим отметку, что клиент успешно обслужен.
                client->SetST( Contact::FINISH, msg );
                closesocket( client->clientSock );
                break;
            }
            printf( "From client: %s\n", msg );
            // Отправляем прочитанное сообщение обратно клиенту.
            send( client->clientSock, msg, sizeof( msg ), NULL );
        }
    }

    // Завершаем обслуживающий поток.
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
