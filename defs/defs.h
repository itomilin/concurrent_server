#pragma once

#include <string>
#include <list>
#include <cstring>

#include <WinSock2.h>
#include <Windows.h>

#pragma comment( lib, "WS2_32.lib" )
#pragma warning ( disable: 4996 )


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
        WORK, // ���� ����� ������� � ��������
        ABORT, // ������������� ������ ���������� �� ���������
        TIMEOUT, // ������������� ������ ���������� �� �������
        FINISH // ������������� ������ ���������� ���������
    } sthread; // ��������� �������������� ������� (������)
    SOCKET      clientSock{}; // ����� ��� ������ ������� � ��������
    SOCKADDR_IN client{}; // ��������� ������
    int32_t     sizeOfClient{}; // ����� prms
    HANDLE      hthread; // handle ������ (��� ��������)
    HANDLE      htimer; // handle �������
    char        msg[50]{}; // ���������
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

typedef std::list<Contact> ListContact; // ������ �����������
