#pragma once

#include <string>
#include <list>
#include <atomic>
#include <cstring>
#include <iostream>

#include <WinSock2.h>
#include <Windows.h>

#pragma comment( lib, "WS2_32.lib" )
#pragma warning ( disable: 4996 )

/**
* ���������� �� ��������.
* ��� ���������� ������ ����������,
* ���������� ����������� ����������� ���������� ��� WINAPI.
*/
static struct Statistics final
{
    // ����� ���������� ����������� � ������ ������ �������.
    std::atomic<uint16_t> numberOfConnections {0};
    /**
    * ����� ���������� ������� � ������������(�� ����� ������� ������ ��������)
    * �� ��� ����� ������ �������
    */
    std::atomic<uint16_t> numberOfRefusals    {0};
    // ���������� ������������� ������� ����������� �������.
    std::atomic<uint16_t> numberOfSuccess     {0};
    // ���������� �������� ����������� �� ������ �������.
    std::atomic<uint16_t> numberOfActive      {0};

    // ���������� ������ �� ����������.
    std::string getStat()
    {
        return "===Stat===\nNumber of active: " + std::to_string( numberOfActive ) +
            "\nNumber of refusals: " + std::to_string( numberOfRefusals ) +
            "\nNumber of success: " + std::to_string( numberOfSuccess ) +
            "\nNumber of connections: " + std::to_string( numberOfConnections );
    }
} statistics;

// ������� ��� �������.
enum TalkersCommand
{
    START,
    STOP,
    EXIT,
    STATISTICS,
    WAIT,
    SHUTDOWN,
    GETCOMMAND
};

struct Contact // ������� ������ �����������
{
    enum TE
    { // ��������� ������� �����������
        EMPTY, // ������ ������� ������ �����������
        ACCEPT, // ��������� (accept), �� �� �������������
        CONTACT // ������� �������������� �������
    } type; // ��� �������� ������ �����������
    enum ST
    { // ��������� �������������� �������
        WORK, // ���� ����� ������� � �������� | ������, ����� ��������� � 1 �� 3 �������������� �������
        ABORT, // ������������� ������ ���������� �� ��������� | ������, ����� ���� ������������ ������� � �������������� �������
        TIMEOUT, // ������������� ������ ���������� �� ������� | ������, ����� ����������� ������
        FINISH // ������������� ������ ���������� ��������� | ������, ����� ��������� ����������� ������� �������������� �������.
    } sthread; // ��������� �������������� ������� (������)
    SOCKET      clientSock{}; // ����� ��� ������ ������� � ��������
    SOCKADDR_IN client{}; // ��������� ������
    int32_t     sizeOfClient{}; // ������ ��������� �������.
    HANDLE      hthread; // handle ������ (��� ��������)
    HANDLE      htimer; // handle �������
    char        msg[256]{}; // ���������
    char        srvname[15]{}; // ������������ �������������� �������

    char* ipAddr{};
    char* port{};

    Contact( TE t = EMPTY, const char* namesrv = "" ) // �����������
    {
        memset( &client, 0, sizeof( client ) );
        sizeOfClient = sizeof( client );
        //ipAddr = inet_ntoa( client.sin_addr );

        type = t;
        strcpy( srvname, namesrv );
    };

    void SetST( ST sth, const char* m = "" )
    {
        sthread = sth;
        strcpy( msg, m );
    }
};

// ������ �����������.
typedef std::list<Contact> ListContact;
