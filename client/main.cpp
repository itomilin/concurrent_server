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
    connSock = socket( AF_INET, SOCK_STREAM, NULL );
    if ( connect( connSock, (SOCKADDR*)&server, sizeof( server ) ) != 0 )
    {
        std::cout << "Error" << std::endl;
        return 1;
    }
    std::cout << "Connected" << std::endl;
    
    char msg[256] {};
    char answer[256] {};
    ZeroMemory( answer, sizeof( answer ) );

    // TODO. Завожу новый буфер, т.к при использовании старого отправляется пустое собщение.
    // выяснить почему так.
    char msg2[256]{};

    // Первое сообщение, установка соединения с обслуживающим сервером. Команда (echo, time, rand)
    std::cout << "Input (echo|time|rand): ";
    std::cin.getline( msg, sizeof( msg ) );
    send( connSock, msg, sizeof( msg ), NULL );

    Sleep( 50 );

    // Ответ от сервера.
    
    recv( connSock, answer, sizeof( answer ), NULL );
    std::cout << "Answer from ServerServiceThread: " << answer << std::endl;
    HANDLE hCleintHandler = NULL;
    // Если запрос введен неверно, завершаем работу клиента.
    if ( std::strcmp( answer, "ErrorInquiry" ) != 0 )
    {
        //memset( msg, '\0', sizeof( msg ) );
        // Если запрос введен верно просим ввести сообщение.
        hCleintHandler = CreateThread( NULL, NULL, (LPTHREAD_START_ROUTINE)ClientHandler, NULL, NULL, NULL );
        do
        {
            std::cout << "Input a message: ";
            std::cin.getline( msg2, sizeof( msg2 ) );
            send( connSock, msg2, sizeof( msg2 ), NULL );
            Sleep( 50 );

        } while ( std::strlen( msg2 ) != 0 );
    }

    /*WaitForSingleObject( hCleintHandler, INFINITE );*/
    TerminateThread( hCleintHandler, 0 );
    CloseHandle( hCleintHandler );

    std::cout << "Enter any key to exit..." << std::endl;
    std::cin.get();
    return 0;
}
