#include <iostream>
#include <string>
#include <cstring>
#include <list>

#include <WinSock2.h>
#include <Windows.h>

#include "accept.h"

#pragma comment( lib, "WS2_32.lib" )

#pragma warning ( disable: 4996 )

CRITICAL_SECTION scListContact;

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
            acceptData.cmd = TalkersCommand::GETCOMMAND;
            squirt = AS_SQUIRT;
            break;
        case TalkersCommand::EXIT:
            break;
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
        recv( c.clientSock, c.msg, sizeof( c.msg ), NULL );
        if ( c.clientSock == INVALID_SOCKET )
        {
            if ( WSAGetLastError() != WSAEWOULDBLOCK )
                throw errorHandler( "accept: ", WSAGetLastError() );
        }
        else
        {
            std::cout << "Connected new client, socket �"
                      << c.clientSock << std::endl;
            rc = true; // �����������
            
            contacts.push_back( c );
            
        }
        LeaveCriticalSection( &scListContact );
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
    u_long nonblk;
    if ( ioctlsocket( serverSock, FIONBIO, &( nonblk = 0 ) ) == SOCKET_ERROR )
        throw errorHandler( "ioctlsocket: ", WSAGetLastError() );

    acceptData.serverSocket = serverSock;
    commandsCycle( acceptData );

    // 6.
    closesocket( serverSock );
    WSACleanup();

    ExitThread( *(DWORD*)cmd ); // ���������� ������ ������
}

DWORD WINAPI dispatchServer( LPVOID cmd ) // ��������
{
        //while ( *((TalkersCommand*)cmd) )
    while( true )
    //while ( *static_cast<TalkersCommand*>( cmd ) == TalkersCommand::EXIT )
    {
        EnterCriticalSection( &scListContact );
        for ( auto& item : contacts )
        {
            // ���� ������� �� ������� ���� ������������, �� ����� �� ����� ������.
            auto htread = ts( const_cast<char*>( item.msg ), ( LPVOID ) & ( item ) );
            
            // ��������� ���� ts ���������� nullptr, ������ ����� �� ��� ������.
            if ( htread != nullptr )
                item.hthread = htread;
            else
            {
                char msg[] = "ErrorInquiry";
                send( item.clientSock, msg, sizeof( msg ), NULL );
                item.SetST( Contact::FINISH, msg );
                closesocket( item.clientSock );
            }
            std::cout << "Client msg: " << item.msg << std::endl;
        }
        LeaveCriticalSection( &scListContact );
        Sleep( 1000 );
    }

    ExitThread( *(DWORD*)cmd );
}

DWORD WINAPI garbageCleaner( LPVOID cmd ) // ��������
{
    while ( true )
    {
        EnterCriticalSection( &scListContact );
        for ( auto it = contacts.begin(); it != contacts.end(); )
        {
            it = it->sthread == Contact::FINISH ?
                contacts.erase( it ) : std::next( it );
            //auto item = *it;
            //if ( item.sthread == Contact::FINISH )
            //    contacts.erase( it );
            
        }
        LeaveCriticalSection( &scListContact );
        Sleep( 1000 );
    }

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
    if ( argc > 1 )
    {
        port = std::stoi( argv[1] );
        std::string name = argv[2];
        dllName = std::wstring( name.begin(), name.end() );
    }
    //--------------------------------------------------------------------------

    InitializeCriticalSection( &scListContact );

    //--------------------------------------------------------------------------

    HMODULE st = LoadLibrary( dllName.c_str() );
    if ( st == nullptr )
        throw std::runtime_error( "[ ERROR] DLL not loaded!!" );

    ts = ( HANDLE( * )( char*, LPVOID ) )GetProcAddress( st, "SSS" );
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

    WaitForSingleObject( hAcceptServer, INFINITE );
    WaitForSingleObject( hDispatchServer, INFINITE );
    WaitForSingleObject( hGarbageCleaner, INFINITE );
    CloseHandle( hAcceptServer );
    CloseHandle( hDispatchServer );
    CloseHandle( hGarbageCleaner );

    //--------------------------------------------------------------------------

    FreeLibrary( st );

    return EXIT_SUCCESS;
}
