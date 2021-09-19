#include <iostream>

#include "accept.h"

CRITICAL_SECTION scListContact;

ListContact contacts;
std::string callSign{};
std::string pipeName{};

// Команда управления для сервера.
volatile auto cmd = TalkersCommand::START;

// Обработка ошибок WINsock2.
std::string errorHandler( const std::string& msg, const int32_t& retCode )
{
    std::string error_msg = "[ ERROR ] " + msg;

    switch ( retCode )
    {
    case WSAEINTR:
        error_msg = "WSAEINTR";
        break;
    default:
        error_msg += "Unhandled error!";
    }

    return error_msg;
}

void commandsCycle( AcceptData& acceptData )
{
    int32_t squirt{ 0 };
    while ( cmd != TalkersCommand::EXIT )
    {
        switch ( cmd )
        {
        case TalkersCommand::START:
            cmd = TalkersCommand::GETCOMMAND;
            break;
        case TalkersCommand::STOP:
            continue;
        //case TalkersCommand::STATISTICS:
        //    break;
        case TalkersCommand::WAIT:
        {
            // Если обслуживающихся на данный момент нет, то подключаем новых.
            if ( statistics.numberInServiceProcess == NULL )
                cmd = TalkersCommand::GETCOMMAND;
            break;
        }
        case TalkersCommand::SHUTDOWN:
        {
            // Если обслуживающихся нет, то завершаем работу сервера.
            if ( statistics.numberInServiceProcess == NULL )
                cmd = TalkersCommand::EXIT;
            break;
        }
        // Даем разрешение на подключение новых клиентов.
        case TalkersCommand::GETCOMMAND:
        {
            squirt = AS_SQUIRT;
            acceptData.squirt = squirt;
            AcceptCycle( acceptData );
            break;
        }
        }
        Sleep( 10 );
    }
}

bool AcceptCycle( AcceptData& acceptData )
{
    bool rc = false;
    Contact c( Contact::ACCEPT, "EchoServer" );
   
    while ( acceptData.squirt-- > 0 && rc == false )
    {
        EnterCriticalSection( &scListContact );
        c.clientSock = accept( acceptData.serverSocket,
                               (SOCKADDR*)&c.client,
                               &c.sizeOfClient );

        if ( c.clientSock == INVALID_SOCKET )
        {
            if ( WSAGetLastError() != WSAEWOULDBLOCK )
                throw errorHandler( "accept: ", WSAGetLastError() );
        }
        else
        {
            ++statistics.numberOfConnections;
            rc = true; // подключился
            c.type = Contact::ACCEPT;
            contacts.push_back( c );
            // Переводим событие в сигнальное состояние.SetEvent
            SetEvent( my_event );
        }
        LeaveCriticalSection( &scListContact );
        Sleep( 10 );
    }

    return rc;
};

bool getRequestFromClient( char*          name,
                           sockaddr*      from,
                           int32_t*       fLen,
                           const SOCKET&  serverSock )
{
    bool rc = false;
    // Позывной сервера. TODO: Вынести в поле класса.
    //constexpr char callSign[] = "helloserver";
    
    // Структура сокета клиента, заполняется при вызове recvfrom.
    while ( recvfrom( serverSock, name, 256, NULL, from, fLen ) != SOCKET_ERROR )
    {
        // Если сообщение является позывным, выходим из цикла и возвращаем true.
        if ( std::strcmp( name, callSign.c_str() ) == NULL )
        {
            rc = true;
            break;
        }
        else
        {
            const char errMsg[] {'\0'};
            putAnswerToClient( errMsg, from, fLen, SERVER_SOCK);
        }
    }

    if ( WSAGetLastError() == WSAETIMEDOUT )
        return rc;
    else if ( WSAGetLastError() == WSAEWOULDBLOCK ) // currently no data available
    {
        //std::cout << WSAGetLastError() << std::endl;
    }
    else if ( WSAGetLastError() != NULL )
        std::cout << WSAGetLastError() << std::endl;
        //throw errorHandler( "Server recfrom: ", WSAGetLastError() );

    return rc;
}

bool putAnswerToClient( const char*     name,
                        const sockaddr* to,
                        const int32_t*  lTo,
                        const SOCKET&   serverSock )
{
     return sendto( serverSock, name, 256/*strlen( name ) + 1*/, NULL, to, *lTo )
         == SOCKET_ERROR ? false : true;
}

DWORD WINAPI acceptServer( LPVOID data )
{
    auto acceptData = *( (AcceptData*)data );

    // Параметры сокета сервера.
    SOCKADDR_IN server{};
    server.sin_addr.S_un.S_addr = INADDR_ANY;
    server.sin_port = htons( acceptData.port );
    server.sin_family = AF_INET;

    // Задаем сокеты для TCP и UDP.
    SOCKET serverSock = socket( AF_INET, SOCK_DGRAM, NULL );
    SERVER_SOCK = serverSock;
    if ( bind( serverSock, (SOCKADDR*)&server, sizeof( server ) ) == SOCKET_ERROR )
        throw errorHandler( "Server bind: ", WSAGetLastError() );

    SOCKET serverSockTCP = socket( AF_INET, SOCK_STREAM, NULL );
    if ( bind( serverSockTCP, (SOCKADDR*)&server, sizeof( server ) ) == SOCKET_ERROR )
        throw errorHandler( "Server bind: ", WSAGetLastError() );
    listen( serverSockTCP, SOMAXCONN );

    // Устанавливаем неблокирующий режим для обоих сокетов.
    u_long nonblk = 1;
    if ( ioctlsocket( serverSock, FIONBIO, &nonblk ) == SOCKET_ERROR )
        throw errorHandler( "ioctlsocket: ", WSAGetLastError() );

    if ( ioctlsocket( serverSockTCP, FIONBIO, &nonblk ) == SOCKET_ERROR )
        throw errorHandler( "ioctlsocket: ", WSAGetLastError() );

    // Создаем событие.
    my_event = CreateEvent( NULL, FALSE, FALSE, NULL );

    // Получаем сетевое имя компьютера для подключения именнованного канала.
    auto hent = gethostbyaddr( (char*)&server.sin_addr,
        sizeof( server.sin_addr ), server.sin_family );
    std::cout << "Hostname:  " << hent->h_name << std::endl;

    acceptData.serverSocket = serverSockTCP;
    commandsCycle( acceptData );

    // Закрываем сокеты и очищаем ресурсы.
    closesocket( serverSock );
    closesocket( serverSockTCP );
    WSACleanup();

    std::cout << "Close ACCEPT server" << std::endl;
    ExitThread( *(DWORD*)data ); // завершение работы потока
}

DWORD WINAPI dispatchServer( LPVOID data )
{
    while( cmd != TalkersCommand::EXIT )
    {
        Sleep( 50 );
        // Сканируем список только когда сработало событие из accept_cycle.
        if ( WaitForSingleObject( my_event, 100 ) == WAIT_OBJECT_0 )
        {
            // Возобновляем событие, пока не будут обслужены клиенты.
            SetEvent( my_event );
            EnterCriticalSection( &scListContact );
            for ( auto& item : contacts )
            {
                /**
                * Поскольку сокет работает в неблокирующем режиме, то когда клиент
                * не вызвал send, значение recv возвращает SOCKET_ERROR. Поэтому как только
                * сообщение было отправлено с клиента, запускаем обслужвающий поток.
                * Также не обрабатываем клиентов, которые уже в режиме обслуживания.
                */
                if ( recv( item.clientSock,
                    item.msg,
                    sizeof( item.msg ), NULL ) != SOCKET_ERROR
                    && item.sthread != Contact::WORK )
                {
                    ++statistics.numberInServiceProcess;
                    /**
                    * Если команда от клиента была неправильная, то поток не будет создан.
                    * Допустимые команды echo|time|rand.
                    */
                    char msg[256] = "ErrorInquiry"; // TODO: must be a field
                    if ( ts( const_cast<char*>( item.msg ), (LPVOID&)( item ) ) != nullptr )
                    {
                        std::strcpy( msg, "Connected to service_server." );
                        send( item.clientSock, msg, sizeof( msg ), NULL );
                    }
                    else
                    {
                        send( item.clientSock, msg, sizeof( msg ), NULL );
                        item.SetST( Contact::ABORT, msg );
                    }
                    Sleep( 10 );
                }
                // Проверяем есть ли не обслуженные клиенты.
                if ( std::find_if( contacts.begin(), contacts.end(),
                    []( const Contact& item )
                    {
                        return item.sthread != Contact::WORK;
                    } ) == contacts.end() )
                {
                    // Сбрасываем событие, когда все обслужены.
                    ResetEvent( my_event );
                }
            }
            LeaveCriticalSection( &scListContact );
        }
    }

    std::cout << "Close DISPATCH" << std::endl;
    ExitThread( *(DWORD*)data );
}

DWORD WINAPI garbageCleaner( LPVOID data ) // прототип
{
    while ( cmd != TalkersCommand::EXIT )
    {
        EnterCriticalSection( &scListContact );
        for ( auto it = contacts.begin(); it != contacts.end(); )
        {
            if ( it->sthread == Contact::FINISH )
            {
                closesocket( it->clientSock );
                CloseHandle( it->hthread );
                std::cout << "Disconnected client (Success): " << it->clientSock << std::endl;
                it = contacts.erase( it );
                ++statistics.numberOfSuccess;
                --statistics.numberInServiceProcess;
            }
            else if ( it->sthread == Contact::TIMEOUT ||
                      it->sthread == Contact::ABORT )
            {
                closesocket( it->clientSock );
                CloseHandle( it->hthread );
                std::cout << "Disconnected client (Refuse): " << it->clientSock << std::endl;
                it = contacts.erase( it );
                ++statistics.numberOfRefusals;
                --statistics.numberInServiceProcess;
            }
            else
            {
                it = std::next( it );
            }
        }
        LeaveCriticalSection( &scListContact );
        Sleep( 10 );
    }

    std::cout << "Close GB cleaner" << std::endl;
    ExitThread( *(DWORD*)data );
}

DWORD WINAPI consolePipe( LPVOID data ) // прототип
{
    constexpr char err_cmd[] {"nocmd"};
    
    HANDLE hPipe{}; // дескриптор канала

    std::wstringstream cls;
    cls << L"\\\\.\\pipe\\" << pipeName.c_str();
    std::wstring total = cls.str();

    // Создаем канал.
    if ( ( hPipe = CreateNamedPipe( total.c_str(),
        PIPE_ACCESS_DUPLEX, //дуплексный канал
        PIPE_TYPE_MESSAGE | PIPE_WAIT, // сообщения|синхронный
        1, NULL, NULL, // максимум 1 экземпляр
        INFINITE, NULL ) ) == INVALID_HANDLE_VALUE )
        throw errorHandler( "Create pipe: ", GetLastError() );
    if ( !ConnectNamedPipe( hPipe, NULL ) ) // ожидать клиента
        throw errorHandler( "Connect pipe: ", GetLastError() );

    while ( cmd != TalkersCommand::EXIT )
    {
        char buf[256] { };
        LPDWORD countReadedBytes {};
        auto answer = ReadFile( hPipe, buf, sizeof( buf ), countReadedBytes, NULL );
        bool is_correct = false;
        if ( answer != FALSE )
        {
            /**
            * Проходим по всем командам, определенным в Enum,
            * чтобы определить корректная ли команда была отправлена.
            */
            for ( std::size_t i = TalkersCommand::START; i <= TalkersCommand::SHUTDOWN; ++i )
            {
                if ( i == (TalkersCommand)std::stoi( buf ) )
                {
                    auto current_command = cmd;
                    // Меняем текущую команду.
                    cmd = (TalkersCommand)std::stoi( buf );

                    // Если вызывается статистика, то алгоритм следующий.
                    if ( cmd == TalkersCommand::STATISTICS )
                    {
                        // Получаем статистику по активным наданный момент.
                        statistics.numberOfActive = static_cast<int16_t>( contacts.size() );
                        // Копируем в буфер информацию структуры.
                        std::strcpy( buf, statistics.getStat().c_str() );
                        // Отправляем сообщение по каналу.
                        WriteFile( hPipe, buf, sizeof( buf ), countReadedBytes, NULL );
                        // Изменяем команду на ту, которая была до этого, т.к статистика одноразовый вызов.
                        cmd = current_command;
                    }
                    else // Во вех других случаях отправляем удаленной консоли эту же команду.
                        WriteFile( hPipe, buf, sizeof( buf ), countReadedBytes, NULL );
                    is_correct = true;
                    break;
                }
            }

            // Если была отправлена команда с неизвестным номером, отправляем nocmd.
            if ( !is_correct )
                WriteFile( hPipe, err_cmd, sizeof( err_cmd ), countReadedBytes, NULL );
        }
    }

    DisconnectNamedPipe( hPipe );
    CloseHandle( hPipe );

    std::cout << "Close PIPE" << std::endl;
    ExitThread( *(DWORD*)data );
}

DWORD WINAPI responseServer( LPVOID data )
{
    while ( cmd != TalkersCommand::EXIT )
    {
        Sleep( 10 );
        // Если команда STOP, запрещаем принимать широковещательные запросы.
        if ( cmd != TalkersCommand::STOP )
        {
            // Если на данный момент есть обслуживающиеся клиенты и WAIT, также не принимаем запросы.
            if ( statistics.numberInServiceProcess != NULL && cmd == TalkersCommand::WAIT )
                continue;
            // Параметры сокета клиента.
            SOCKADDR_IN client{};
            char buf[256]{};
            int32_t sizeOfClient = sizeof( client );
            int32_t acceptedBytes{};

            if ( getRequestFromClient( buf, (sockaddr*)&client, &sizeOfClient, SERVER_SOCK ) )
            {
                std::cout << "Request was recieved..." << std::endl;
                if ( putAnswerToClient( buf, (sockaddr*)&client, &sizeOfClient, SERVER_SOCK ) )
                {
                    std::cout << "Answer was sent..." << std::endl;
                }
            }
        }
    }

    std::cout << "Close responseServer." << std::endl;
    ExitThread( *(DWORD*)data );
}

int main( int argc, char** argv )
{
    // 1. Инициализация библиотеки WINsock2.
    WSADATA wsaData;
    if ( WSAStartup( MAKEWORD( 2, 0 ), &wsaData ) == EXIT_SUCCESS )
        std::cout << "[ OK ] Init WSAStartup!" << std::endl;
    else
        throw errorHandler( "WSAStartup: ", WSAGetLastError() );

    // Если в параметрах командной строки не установлен порт, по умолчанию 2000;
    int16_t UDPPort { 2000 };
    std::wstring dllName = std::wstring( L"service_library" );
    std::string name = "ConsolePipe";
    callSign = "helloserver";

    if ( argc > 4 )
    {
        UDPPort = std::stoi( argv[1] );
        std::string name = argv[2];
        dllName = std::wstring( name.begin(), name.end() );
        pipeName = argv[3];
        callSign = argv[4];
    }
    else
    {
        std::cout << "[ ERROR] Check args." << std::endl;
        std::exit( EXIT_FAILURE );
    }

    std::cout << "Params for connect r_console and clients:" << std::endl
        << "UDP_port:  " << std::to_string( UDPPort ) << std::endl
        << "Pipe_name: " << pipeName << std::endl
        << "Call_sign: " << callSign << std::endl;

    //--------------------------------------------------------------------------

    InitializeCriticalSection( &scListContact );

    //--------------------------------------------------------------------------
    
    // Загрузка DLL.
    HMODULE st = LoadLibrary( dllName.c_str() );
    if ( st == nullptr )
        throw std::runtime_error( "[ ERROR] DLL not loaded!!" );

    ts = ( HANDLE( * )( const char*, LPVOID& ) )GetProcAddress( st, "SSS" );
    if ( ts == nullptr )
        throw std::runtime_error( "[ ERROR] Import DLL function!!" );

    //--------------------------------------------------------------------------

    // Создание потоков и установка приоритетов.
    AcceptData acceptData {};
    acceptData.cmd = cmd;
    acceptData.port = UDPPort;

    hAcceptServer = CreateThread( NULL, NULL,
        (LPTHREAD_START_ROUTINE)acceptServer,
        (LPVOID)&acceptData, NULL, NULL );

    if ( hAcceptServer == nullptr )
        throw std::runtime_error( "[ ERROR ] Thread AcceptServer was not created!!" );
    if ( SetThreadPriority( hAcceptServer, THREAD_PRIORITY_ABOVE_NORMAL ) == FALSE )
        throw std::runtime_error( "[ ERROR ] Can`t set priority!!" );

    //--------------------------------------------------------------------------

    hDispatchServer = CreateThread( NULL, NULL,
        (LPTHREAD_START_ROUTINE)dispatchServer,
        (LPVOID)&acceptData, NULL, NULL );

    if ( hDispatchServer == nullptr )
        throw std::runtime_error( "[ ERROR ] Thread DispatchServer was not created!!" );
    if ( SetThreadPriority( hDispatchServer, THREAD_PRIORITY_NORMAL ) == FALSE )
        throw std::runtime_error( "[ ERROR ] Can`t set priority!!" );

    //--------------------------------------------------------------------------

    hGarbageCleaner = CreateThread( NULL, NULL,
        (LPTHREAD_START_ROUTINE)garbageCleaner,
        (LPVOID)&acceptData, NULL, NULL );

    if ( hGarbageCleaner == nullptr )
        throw std::runtime_error( "[ ERROR ] Thread GarbageCleaner was not created!!" );
    if ( SetThreadPriority( hGarbageCleaner, THREAD_PRIORITY_IDLE ) == FALSE )
        throw std::runtime_error( "[ ERROR ] Can`t set priority!!" );

    //--------------------------------------------------------------------------

    hConsolePipe = CreateThread( NULL, NULL,
        (LPTHREAD_START_ROUTINE)consolePipe,
        (LPVOID)&acceptData, NULL, NULL );

    if ( hConsolePipe == nullptr )
        throw std::runtime_error( "[ ERROR ] Thread ConsolePipe was not created!!" );
    if ( SetThreadPriority( hConsolePipe, THREAD_PRIORITY_NORMAL ) == FALSE )
        throw std::runtime_error( "[ ERROR ] Can`t set priority!!" );

    //--------------------------------------------------------------------------

    Sleep( 100 );

    hResponseServer = CreateThread( NULL, NULL,
        (LPTHREAD_START_ROUTINE)responseServer,
        (LPVOID)&acceptData, NULL, NULL );

    if ( hResponseServer == nullptr )
        throw std::runtime_error( "[ ERROR ] Thread ResponseServer was not created!!" );
    if ( SetThreadPriority( hResponseServer, THREAD_PRIORITY_NORMAL ) == FALSE )
        throw std::runtime_error( "[ ERROR ] Can`t set priority!!" );

    //--------------------------------------------------------------------------

    WaitForSingleObject( hAcceptServer,   INFINITE );    
    WaitForSingleObject( hDispatchServer, INFINITE );
    WaitForSingleObject( hGarbageCleaner, INFINITE );
    WaitForSingleObject( hConsolePipe,    INFINITE );
    WaitForSingleObject( hResponseServer, INFINITE );

    CloseHandle( hAcceptServer   );
    CloseHandle( hDispatchServer );
    CloseHandle( hGarbageCleaner );
    CloseHandle( hConsolePipe    );
    CloseHandle( hResponseServer );

    //--------------------------------------------------------------------------

    FreeLibrary( st );

    std::cout << "<<<The server has shutdown>>>" << std::endl;

    return EXIT_SUCCESS;
}
