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
    // �����������.
    Client( const int16_t& port );

    // ����� ������� ��������� ����������������� �����.
    void find_server();

    // �������� �������� � ������������� �������� �� TCP.
    void connect_to_service_server();

    // ����������.
    ~Client();

private:

    const char* _error_command{"ErrorInquiry"};

    // ����� ������������� ������� � �� ����������.
    bool _init_sockets();

    // ����� ��� ��������� ������ ������� ��� ����������� ���������� � ���.
    bool _get_server( char* call,
                      const SOCKET& sock_brd,
                      SOCKADDR* from,
                      int32_t* f_len );

    // ����.
    const int16_t _port{};

    // ������ ��� �������� ������� � ������.
    char _msg_to[256]{};
    char _msg_from[256]{};

    // ����� ��� TCP.
    SOCKET _sock_TCP{};

    // ����� ��� ������������������ ������.
    SOCKET _sock_brd{};

    // ��������� ������ �������, �� �������� ������� �����.
    SOCKADDR_IN _server{};
    int32_t _f_len = sizeof( _server );

    // ��������� ������ ������������������ ������.
    SOCKADDR_IN _all{};

};
