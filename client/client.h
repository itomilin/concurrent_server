#pragma once

#include <iostream>
#include <string>
#include <cstring>

#include <WinSock2.h>
#include <Windows.h>

#pragma comment( lib, "WS2_32.lib" )
#pragma warning ( disable: 4996 )

class Client final
{
public:
    // Конструктор.
    Client( const int16_t& port );

    // Поиск сервера отправляя широковещательный пакет.
    void find_server();

    // Начинаем общаться с обслуживающим сервером по TCP.
    void connect_to_service_server();

    // Деструктор.
    ~Client();

private:

    const char* _error_command{"ErrorInquiry"};

    // Метод инициализации сокетов и их параметров.
    bool _init_sockets();

    // Метод для получения данных сервера для дальнейшего соединения с ним.
    bool _get_server( char* call,
                      const SOCKET& sock_brd,
                      SOCKADDR* from,
                      int32_t* f_len );

    // Порт.
    const int16_t _port{};

    // Буферы для хранения запроса и ответа.
    char _msg_to[256]{};
    char _msg_from[256]{};

    // Сокет для TCP.
    SOCKET _sock_TCP{};

    // Сокет для широковещательного пакета.
    SOCKET _sock_brd{};

    // Структура сокета сервера, от которого получем ответ.
    SOCKADDR_IN _server{};
    int32_t _f_len = sizeof( _server );

    // Структура сокета широковещательного пакета.
    SOCKADDR_IN _all{};

};
