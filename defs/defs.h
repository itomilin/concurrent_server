#pragma once

#include <string>
#include <list>
#include <cstring>

#include <WinSock2.h>
#include <Windows.h>

#pragma comment( lib, "WS2_32.lib" )
#pragma warning ( disable: 4996 )

//  оманды дл€ сервера.
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

struct Contact // элемент списка подключений
{
    enum TE
    { // состо€ние сервера подключени€
        EMPTY, // пустой элемент списка подключений
        ACCEPT, // подключен (accept), но не обслуживаетс€
        CONTACT // передан обслуживающему серверу
    } type; // тип элемента списка подключений
    enum ST
    { // состо€ние обслуживающего сервера
        WORK, // идет обмен данными с клиентом
        ABORT, // обслуживающий сервер завершилс€ не нормально
        TIMEOUT, // обслуживающий сервер завершилс€ по времени
        FINISH // обслуживающий сервер завершилс€ нормально
    } sthread; // состо€ние обслуживающего сервера (потока)
    SOCKET      clientSock{}; // сокет дл€ обмена данными с клиентом
    SOCKADDR_IN client{}; // параметры сокета
    int32_t     sizeOfClient{}; // длина prms
    HANDLE      hthread; // handle потока (или процесса)
    HANDLE      htimer; // handle таймера
    char        msg[50]{}; // сообщение
    char        srvname[15]{}; // наименование обслуживающего сервера

    char* ipAddr{};
    char* port{};

    Contact( TE t = EMPTY, const char* namesrv = "" ) // конструктор
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

typedef std::list<Contact> ListContact; // список подключений
