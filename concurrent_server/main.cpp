#include <iostream>

#include "accept.h"


CRITICAL_SECTION scListContact;

ListContact contacts;

// ��������� ������ WINsock2.
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
    while ( acceptData.cmd != TalkersCommand::EXIT )
    {
        switch ( acceptData.cmd )
        {
        case TalkersCommand::START:
            acceptData.cmd = TalkersCommand::GETCOMMAND;
            squirt = AS_SQUIRT;
            break;
        case TalkersCommand::STOP:
            continue;
            //break;
        //case TalkersCommand::EXIT:
        //    break;
        case TalkersCommand::STATISTICS:
            break;
        case TalkersCommand::WAIT:
            break;
        case TalkersCommand::SHUTDOWN:
            break;
        case TalkersCommand::GETCOMMAND:
            break;
        }

        //std::cout << "Accepted cmd: " << acceptData.cmd << std::endl;
        acceptData.squirt = squirt;

        if ( AcceptCycle( acceptData ) )
        {
            acceptData.cmd = TalkersCommand::GETCOMMAND;
        }
        else
        {
            SleepEx( 0, TRUE );
        }
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
        //auto t = recv( c.clientSock, c.msg, sizeof( c.msg ), NULL );

        if ( c.clientSock == INVALID_SOCKET )
        {
            if ( WSAGetLastError() != WSAEWOULDBLOCK )
                throw errorHandler( "accept: ", WSAGetLastError() );
        }
        else
        {
            //std::cout << "Connected new client, socket #"
            //          << c.clientSock << std::endl;
            rc = true; // �����������
            c.type = Contact::ACCEPT;
            contacts.push_back( c );
            
        }
        LeaveCriticalSection( &scListContact );
        Sleep( 100 );
    }
    
    return rc;
};

DWORD WINAPI acceptServer( LPVOID cmd ) // ��������
{
    auto acceptData = *( (AcceptData*)cmd );
    constexpr char ip[] = "127.0.0.1";

    // 2.
    SOCKET serverSock = socket( AF_INET, SOCK_STREAM, NULL );

    SOCKADDR_IN server;
    int32_t sizeOfServer = sizeof( server );
    server.sin_addr.S_un.S_addr = inet_addr( ip );
    server.sin_port = htons( acceptData.port );
    server.sin_family = AF_INET;

    bind( serverSock, (LPSOCKADDR)&server, sizeof( server ) );
    listen( serverSock, SOMAXCONN );

    // 3.
    u_long nonblk = 1;
    if ( ioctlsocket( serverSock, FIONBIO, &nonblk ) == SOCKET_ERROR )
        throw errorHandler( "ioctlsocket: ", WSAGetLastError() );

    acceptData.serverSocket = serverSock;
    commandsCycle( acceptData );



    // 6.
    closesocket( serverSock );
    WSACleanup();

    ExitThread( *(DWORD*)cmd ); // ���������� ������ ������
}

DWORD WINAPI dispatchServer( LPVOID data ) // ��������
{
    //while ( static_cast<AcceptData*>( data )->cmd != TalkersCommand::EXIT )
    while(true)
    {
        EnterCriticalSection( &scListContact );
        for ( auto& item : contacts )
        {
            /**
            * ��������� ����� �������� � ������������� ������, �� ����� ������
            * �� ������ send, �������� recv ���������� SOCKET_ERROR. ������� ��� ������
            * ��������� ���� ���������� � �������, ��������� ������������ �����.
            * ����� �� ������������ ��������, ������� ��� � ������ ������������.
            */
            if ( recv( item.clientSock,
                       item.msg,
                       sizeof( item.msg ), NULL ) != SOCKET_ERROR
                && item.sthread != Contact::WORK )
            {
                //item.htimer = CreateWaitableTimer( NULL, FALSE, L"timer1" );
                //if ( NULL == item.htimer )
                //    printf( "CreateWaitableTimer failed (%d)\n", GetLastError() );

                //LARGE_INTEGER fTime;
                //fTime.QuadPart = -60000000LL;
                //if ( !SetWaitableTimer( item.htimer, &fTime, NULL,
                //    ASFinishMessage, (LPVOID*)&item, FALSE ) )
                //    printf( "SetWaitableTimer failed (%d)\n", GetLastError() );

                //SleepEx( INFINITE, TRUE );
                /*if ( WaitForSingleObject( htimer, INFINITE ) != WAIT_OBJECT_0 )
                    printf( "WaitForSingleObject failed (%d)\n", GetLastError() );
                else
                    printf( "Timer was signaled.\n" );*/
                /**
                * ���� ������� �� ������� ���� ������������, �� ����� �� ����� ������.
                * ���������� ������� echo|time|rand.
                */
                
                auto htread = ts( const_cast<char*>( item.msg ), (LPVOID&)( item ) );

                //// ��������� ���� ts ���������� nullptr, ������ ����� �� ��� ������.
                char msg[256] = "ErrorInquiry";
                if ( htread != nullptr )
                {
                    std::strcpy( msg, "Connected to service_server." );
                    send( item.clientSock, msg, sizeof( msg ), NULL );
                    item.hthread = htread;
                }
                else
                {
                    send( item.clientSock, msg, sizeof( msg ), NULL );
                    item.SetST( Contact::FINISH, msg );
                    closesocket( item.clientSock );
                }
                //std::cout << "Client msg: " << item.msg << std::endl;
            }
        }
        LeaveCriticalSection( &scListContact );
        Sleep( 100 );
    }

    std::cout << "Close DISPATCH\n";
    ExitThread( *(DWORD*)data );
}

DWORD WINAPI garbageCleaner( LPVOID cmd ) // ��������
{
    while ( true )
    {
        EnterCriticalSection( &scListContact );
        for ( auto it = contacts.begin(); it != contacts.end(); )
        {
            auto test = it->sthread;
            it = it->sthread == Contact::FINISH ?
                contacts.erase( it ) : std::next( it );
            //auto item = *it;
            //if ( item.sthread == Contact::FINISH )
            //    contacts.erase( it );
            
        }
        LeaveCriticalSection( &scListContact );
        Sleep( 100 );
    }

    ExitThread( *(DWORD*)cmd );
}

DWORD WINAPI consolePipe( LPVOID cmd ) // ��������
{
    std::cout << "BEFORE: " << ( (AcceptData*)cmd )->cmd << std::endl;
    HANDLE hPipe; // ���������� ������

    try
    {
        if ( ( hPipe = CreateNamedPipe(L"\\\\.\\pipe\\ConsolePipe",
            PIPE_ACCESS_DUPLEX, //���������� �����
            PIPE_TYPE_MESSAGE | PIPE_WAIT, // ���������|���������� // NOWAIT
            1, NULL, NULL, // �������� 1 ���������
            INFINITE, NULL ) ) == INVALID_HANDLE_VALUE )
            throw errorHandler( "create:", GetLastError() );
        if ( !ConnectNamedPipe( hPipe, NULL ) ) // ������� �������
            throw errorHandler( "connect:", GetLastError() );
        

    }
    catch ( std::string ErrorPipeText )
    {
        std::cout << std::endl << ErrorPipeText;
    }

    while ( true )
    {
        char buf[256]{ "\0" };
        LPDWORD countReadedBytes{};
        auto answer = ReadFile( hPipe, buf, sizeof( buf ), countReadedBytes, NULL );
        if ( answer != 0 )
        {
            ( (AcceptData*)cmd )->cmd = (TalkersCommand)std::stoi( buf );
            std::cout << "AFTER: " << ( (AcceptData*)cmd )->cmd << std::endl;
        }
        //else
        //    DisconnectNamedPipe( hPipe );
    }

    DisconnectNamedPipe( hPipe );
    CloseHandle( hPipe );
    ExitThread( *(DWORD*)cmd );
}

int main( int argc, char** argv )
{
    // 1. ������������� ���������� WINsock2.
    WSADATA wsaData;
    if ( WSAStartup( MAKEWORD( 2, 0 ), &wsaData ) == EXIT_SUCCESS )
        std::cout << "[ OK ] Init WSAStartup!" << std::endl;
    else
        throw errorHandler( "WSAStartup: ", WSAGetLastError() );
    // ���� � ���������� ��������� ������ �� ���������� ����, �� ��������� 2000;
    int16_t port{ 2000 };
    std::wstring dllName = std::wstring( L"service_library" );
    std::string pipeName = "\\\\.\\pipe\\ConsolePipe";

    if ( argc > 1 )
    {
        port = std::stoi( argv[1] );
        std::string name = argv[2];
        dllName = std::wstring( name.begin(), name.end() );
        pipeName = argv[3];
    }
    //--------------------------------------------------------------------------

    InitializeCriticalSection( &scListContact );

    //--------------------------------------------------------------------------

    HMODULE st = LoadLibrary( dllName.c_str() );
    if ( st == nullptr )
        throw std::runtime_error( "[ ERROR] DLL not loaded!!" );

    ts = ( HANDLE( * )( const char*, LPVOID& ) )GetProcAddress( st, "SSS" );
    if ( ts == nullptr )
        throw std::runtime_error( "[ ERROR] Import DLL function!!" );
    
    // ������� ���������� ��� �������.
    volatile auto cmd = TalkersCommand::START;

    //--------------------------------------------------------------------------

    AcceptData acceptData;
    acceptData.cmd = cmd;
    acceptData.port = port;

    hAcceptServer = CreateThread( NULL, NULL,
        (LPTHREAD_START_ROUTINE)acceptServer,
        (LPVOID)&acceptData, NULL, NULL );

    if ( hAcceptServer == nullptr )
        throw std::runtime_error( "[ ERROR ] Thread AcceptServer was not created!!" );

    //--------------------------------------------------------------------------

    hDispatchServer = CreateThread( NULL, NULL,
        (LPTHREAD_START_ROUTINE)dispatchServer,
        (LPVOID)&acceptData, NULL, NULL );

    if ( hDispatchServer == nullptr )
        throw std::runtime_error( "[ ERROR ] Thread DispatchServer was not created!!" );

    //--------------------------------------------------------------------------

    hGarbageCleaner = CreateThread( NULL, NULL,
        (LPTHREAD_START_ROUTINE)garbageCleaner,
        (LPVOID)&acceptData, NULL, NULL );

    if ( hGarbageCleaner == nullptr )
        throw std::runtime_error( "[ ERROR ] Thread GarbageCleaner was not created!!" );

    //--------------------------------------------------------------------------

    hConsolePipe = CreateThread( NULL, NULL,
        (LPTHREAD_START_ROUTINE)consolePipe,
        (LPVOID)&acceptData, NULL, NULL );

    if ( hConsolePipe == nullptr )
        throw std::runtime_error( "[ ERROR ] Thread ConsolePipe was not created!!" );

    //--------------------------------------------------------------------------

    WaitForSingleObject( hAcceptServer, INFINITE );
    WaitForSingleObject( hDispatchServer, INFINITE );
    WaitForSingleObject( hGarbageCleaner, INFINITE );
    WaitForSingleObject( hConsolePipe, INFINITE );

    CloseHandle( hAcceptServer );
    CloseHandle( hDispatchServer );
    CloseHandle( hGarbageCleaner );
    CloseHandle( hConsolePipe );

    //--------------------------------------------------------------------------

    FreeLibrary( st );

    return EXIT_SUCCESS;
}
