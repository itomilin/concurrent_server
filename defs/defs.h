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
* —татистика по клиентам.
* ¬се переменные делаем атомарными,
* используем возможности стандартной библиотеки без WINAPI.
*/
static struct Statistics final
{
    // ќбщее количество подключений с начала работы сервера.
    std::atomic<uint16_t> numberOfConnections {0};
    /**
    * ќбщее количество отказов в обслуживании(по всему спектру причин суммарно)
    * за все врем€ работы сервера
    */
    std::atomic<uint16_t> numberOfRefusals    {0};
    //  оличество подсоединений которые завершились успешно.
    std::atomic<uint16_t> numberOfSuccess     {0};
    //  оличество активных подключений на момент запроса.
    std::atomic<uint16_t> numberOfActive      {0};

    // ¬озвращаем строку со счетчиками.
    std::string getStat()
    {
        return "===Stat===\nNumber of active: " + std::to_string( numberOfActive ) +
            "\nNumber of refusals: " + std::to_string( numberOfRefusals ) +
            "\nNumber of success: " + std::to_string( numberOfSuccess ) +
            "\nNumber of connections: " + std::to_string( numberOfConnections );
    }
} statistics;

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
        WORK, // идет обмен данными с клиентом | ставим, когда подключен к 1 из 3 обслуживающему серверу
        ABORT, // обслуживающий сервер завершилс€ не нормально | ставим, когда была неправильна€ команда к обслуживающему серверу
        TIMEOUT, // обслуживающий сервер завершилс€ по времени | ставим, когда срабатывает таймер
        FINISH // обслуживающий сервер завершилс€ нормально | ставим, когда отправили завершающую команду обслуживающему серверу.
    } sthread; // состо€ние обслуживающего сервера (потока)
    SOCKET      clientSock{}; // сокет дл€ обмена данными с клиентом
    SOCKADDR_IN client{}; // параметры сокета
    int32_t     sizeOfClient{}; // –азмер структуры клиента.
    HANDLE      hthread; // handle потока (или процесса)
    HANDLE      htimer; // handle таймера
    char        msg[256]{}; // сообщение
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

// —писок подключений.
typedef std::list<Contact> ListContact;
