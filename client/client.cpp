#include "client.h"

Client::Client( const int16_t& port )
    : _port ( port )
{
    WSADATA wsa_data{};
    // Инициализация библиотеки WINsock2.
    if ( WSAStartup( MAKEWORD( 2, 0 ), &wsa_data ) == EXIT_SUCCESS )
        std::cout << "[ OK ] Init WSAStartup!" << std::endl;
    
    if ( _init_sockets() )
        std::cout << "[ OK ] Init Sockets!" << std::endl;
}

Client::~Client()
{
    if ( WSACleanup() != SOCKET_ERROR )
        std::cout << "WSACleanup success" << std::endl;
}

bool Client::_init_sockets()
{
    // Параметры сокета для широковещательного пакета.
    _all.sin_addr.s_addr = INADDR_BROADCAST;
    _all.sin_port = htons( _port );
    _all.sin_family = AF_INET;
    int32_t sentBytes{};

    _sock_TCP = socket( AF_INET, SOCK_STREAM, NULL );
    _sock_brd = socket( AF_INET, SOCK_DGRAM, NULL );

    constexpr int optval = 1;
    if ( setsockopt( _sock_brd, SOL_SOCKET, SO_BROADCAST, (char*)&optval, sizeof( int ) == SOCKET_ERROR ) )
        throw "Setsockopt error";

    return true;
}

bool Client::_get_server( char* call,
                          const SOCKET& sock_brd,
                          SOCKADDR* from,
                          int32_t* f_len )
{
    // CRASH WITH strlen( call ) + 1
    auto rc = recvfrom( sock_brd, call, 256, NULL, from, f_len );
    if ( rc == SOCKET_ERROR )
    {
        if ( WSAGetLastError() == WSAETIMEDOUT )
            return false;
        else
            throw "recvfrom error" + WSAGetLastError();
    }

    if ( std::strlen( call ) == NULL )
        return false;

    return true;
}

void Client::connect_to_service_server()
{
    // После того, как был найден сервер, можно общаться по TCP.
    if ( connect( _sock_TCP, (SOCKADDR*)&_server, sizeof( _server ) ) != 0 )
        throw "connect error" + WSAGetLastError();

    std::cout << "Input (echo|time|rand): ";
    std::cin.getline( _msg_to, sizeof( _msg_to ) );
    send( _sock_TCP, _msg_to, sizeof( _msg_to ), NULL );

    recv( _sock_TCP, _msg_from, sizeof( _msg_from ), NULL );
    std::cout << "Answer: " << _msg_from << std::endl;

    // Если команда введена неверно, завершаем работу клиента.
    if ( std::strcmp( _msg_from, _error_command ) != 0 )
    {
        do
        {
            // Повторно чистим буфер ответа, на всякий случай.
            memset( _msg_from, 0x00, sizeof( _msg_from ) );
            // Вводим и отправляем сообщение.
            std::cout << "Input a message: ";
            std::cin.getline( _msg_to, sizeof( _msg_to ) );
            send( _sock_TCP, _msg_to, sizeof( _msg_to ), NULL );
            Sleep( 50 );

            // Получаем ответ и выводим его.
            recv( _sock_TCP, _msg_from, sizeof( _msg_from ), NULL );
            std::cout << "Answer from ServiceServer: " << _msg_from << std::endl;
            Sleep( 50 );
        } while ( std::strlen( _msg_from ) != 0 );
    }
}

void Client::find_server()
{
    // Отправляем широковещательный пакет, пока не будет найден сервер, который откликнется.
    while ( true )
    {
        // Первое сообщение, установка соединения с обслуживающим сервером. Команда (echo, time, rand)
        std::cout << "Input call sign (broadcast): ";
        std::cin.getline( _msg_to, sizeof( _msg_to ) );

        if ( sendto( _sock_brd, _msg_to, sizeof( _msg_to ),
            NULL, (SOCKADDR*)&_all, sizeof( _all ) ) == SOCKET_ERROR )
            throw "SendTo Error";
        else
        {
            if ( _get_server( _msg_to, _sock_brd, (SOCKADDR*)&_server, &_f_len ) )
            {
                std::cout << "!!CONNECTED!!" << std::endl;
                std::cout << "Answer from server: " << _msg_to << std::endl;
                break;
            }
            else
            {
                std::cout << "Wrong call sign. Not CONNECTED." << std::endl;
            }
        }
    }
}
