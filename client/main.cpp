#include <iostream>
#include <string>
#include <cstring>

#include <WinSock2.h>
#include <Windows.h>

#pragma comment( lib, "WS2_32.lib" )
#pragma warning ( disable: 4996 )

int main()
{
    constexpr char ip[] = "127.0.0.1";
    // 1. Инициализация библиотеки WINsock2.
    WSADATA wsaData;
    if ( WSAStartup( MAKEWORD( 2, 0 ), &wsaData ) == EXIT_SUCCESS )
        std::cout << "[ OK ] Init WSAStartup!" << std::endl;

    // 2.
//    SOCKET serverSock = socket( AF_INET, SOCK_STREAM, NULL );

    SOCKADDR_IN server;
    server.sin_addr.s_addr = inet_addr( ip );
    server.sin_port = htons( 2000 );
    server.sin_family = AF_INET;

    // 3.
    SOCKET connSock = socket( AF_INET, SOCK_STREAM, NULL );
    if ( connect( connSock, (SOCKADDR*)&server, sizeof( server ) ) != 0 )
    {
        std::cout << "Error" << std::endl;
        return 1;
    }
    std::cout << "Connected" << std::endl;
    
    char msg[256] {};
    char msg2[256]{};

    while ( true )
    {
        std::cout << "Input msg: ";
        std::cin.getline( msg, sizeof( msg ) );
        send( connSock, msg, sizeof( msg ), NULL );

        std::cout << "Answer: ";
        recv( connSock, msg2, sizeof( msg2 ), NULL );
        std::cout << msg2 << std::endl;
    }

    std::cin.get();
    return 0;
}
