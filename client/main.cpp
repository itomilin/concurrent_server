#include <iostream>
#include <string>
#include <cstring>

#include <WinSock2.h>
#include <Windows.h>

#pragma comment( lib, "WS2_32.lib" )
#pragma warning ( disable: 4996 )
SOCKET connSock;


class Client final
{
public:
    Client( const int16_t& port );
    ~Client();

    void SendMessage( const std::string& msg );

private:

};


void ClientHandler()
{
    char answer[256];
    while ( true )
    {
        recv( connSock, answer, sizeof( answer ), NULL );
        std::cout << "Answer from ServerServiceThread: " << answer << std::endl;
        Sleep( 50 );
    }
}

bool getServer( char*         call,
                const SOCKET& connSock,
                SOCKADDR*     from,
                int32_t*      fLen )
{
    auto rc = recvfrom( connSock, call, 256/* strlen( call ) + 1*/, NULL, from, fLen );
    if ( rc == SOCKET_ERROR )
    {
        if ( WSAGetLastError() == WSAETIMEDOUT )
            return false;
        else
            throw "recvfrom error" + WSAGetLastError();
    }

    if ( std::strlen( call ) == NULL )
    {
        return false;
    }

    return true;
}

int main()
{
    // 1. Инициализация библиотеки WINsock2.
    WSADATA wsaData;
    if ( WSAStartup( MAKEWORD( 2, 0 ), &wsaData ) == EXIT_SUCCESS )
        std::cout << "[ OK ] Init WSAStartup!" << std::endl;

    // Параметры сокета для широковещательного пакета.
    SOCKADDR_IN all {};
    all.sin_addr.s_addr = INADDR_BROADCAST;
    all.sin_port = htons( 2000 );
    all.sin_family = AF_INET;
    int32_t sentBytes {};

    // Параметры сокета сервера, от которого придет ответ.
    SOCKADDR_IN server {};
    int32_t fLen = sizeof( server );

    // 3.
    connSock = socket( AF_INET, SOCK_DGRAM, NULL );
    
    int optval = 1;
    if ( setsockopt( connSock, SOL_SOCKET, SO_BROADCAST, (char*)&optval, sizeof( int ) == SOCKET_ERROR ) )
        throw "Setsockopt error";

    char msg[256] {};
    char answer[256] {};
    ZeroMemory( answer, sizeof( answer ) );

    // TODO. Завожу новый буфер, т.к при использовании старого отправляется пустое собщение.
    // выяснить почему так.
    char msg2[256] {};

    // Первое сообщение, установка соединения с обслуживающим сервером. Команда (echo, time, rand)
    std::cout << "Input call sign (broadcast): ";
    std::cin.getline( msg, sizeof( msg ) );

    while ( true )
    {
        if ( sendto( connSock, msg, sizeof( msg ),
            NULL, (SOCKADDR*)&all, sizeof( all ) ) == SOCKET_ERROR )
            throw "SendTo Error";
        else
        {
            if ( getServer( msg, connSock, (SOCKADDR*)&server, &fLen ) )
            {
                std::cout << "Answer from server: " << msg << ""/*server.sin_addr.S_un.S_un_b.s_b1*/ << std::endl;
                break;
            }
            else
            {
                std::cout << "Wrong call sign. Not CONNECTED." << std::endl;
            }
        }

        std::cout << "Input call sign: ";
        std::cin.getline( msg, sizeof( msg ) );
    }

    std::cout << "!!CONNECTED!!" << std::endl;
    std::cin.get();
    // Ответ от сервера.
    SOCKET connSockTCP;
    
    connSockTCP = socket( AF_INET, SOCK_STREAM, NULL );
    if ( connect( connSockTCP, (SOCKADDR*)&server, sizeof( server ) ) != 0 )
   // if ( connect( connSockTCP, (SOCKADDR*)&server, sizeof( server ) ) != 0 )
    {
        std::cout << "Error" << std::endl;
        return 1;
    }
    std::cout << "Connected" << std::endl;

    std::cout << "Input (echo|time|rand): ";
    std::cin.getline( msg, sizeof( msg ) );
    send( connSockTCP, msg, sizeof( msg ), NULL );

    recv( connSockTCP, answer, sizeof( answer ), NULL );
    std::cout << "Answer: " << answer;
    //std::cin.getline( msg, sizeof( msg ) );
    //if ( sendto( connSock, msg, sizeof( msg ),
    //    NULL, (SOCKADDR*)&server, sizeof( server ) ) != SOCKET_ERROR )
    //{
    //    if ( getServer( msg, connSock, (SOCKADDR*)&server, &fLen ) )
    //    {
    //        std::cout << "Answer from server: " << msg << ""/*server.sin_addr.S_un.S_un_b.s_b1*/ << std::endl;
    //    }
    //}
    //Sleep( INFINITE );
    //recv( connSock, answer, sizeof( answer ), NULL );

    std::cout << "Answer from ServerServiceThread: " << answer << std::endl;
    HANDLE hCleintHandler = NULL;
    // Если запрос введен неверно, завершаем работу клиента.
    if ( std::strcmp( msg, "ErrorInquiry" ) != 0 )
    {
        //memset( msg, '\0', sizeof( msg ) );
        // Если запрос введен верно просим ввести сообщение.
    /*    hCleintHandler = CreateThread( NULL, NULL, (LPTHREAD_START_ROUTINE)ClientHandler, NULL, NULL, NULL );
        do
        {
            std::cout << "Input a message: ";
            std::cin.getline( msg2, sizeof( msg2 ) );
            send( connSock, msg2, sizeof( msg2 ), NULL );
            Sleep( 50 );

        } while ( std::strlen( msg2 ) != 0 );*/
    }

    /*WaitForSingleObject( hCleintHandler, INFINITE );*/
    TerminateThread( hCleintHandler, 0 );
    CloseHandle( hCleintHandler );

    std::cout << "Enter any key to exit..." << std::endl;
    std::cin.get();
    return 0;
}
