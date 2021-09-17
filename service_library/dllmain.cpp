// dllmain.cpp : Defines the entry point for the DLL application.
#include "pch.h"

//#include "DefineTableService.h" // макро для TableService
//#include "PrototypeService.h" // прототипы обслуживающих потоков

BEGIN_TABLESERVICE
ENTRYSERVICE( "echo", EchoServer ),
ENTRYSERVICE( "time", TimeServer ),
ENTRYSERVICE( "rand", RandServer )
END_TABLESERVICE;

extern "C" __declspec( dllexport ) HANDLE SSS( const char* id, LPVOID& item )
{
    HANDLE rc = NULL;

    int i = 0;

    while ( i < SIZETS && strcmp( TABLESERVICE_ID( i ), id ) != 0 )
        i++;

    if ( i < SIZETS )
    {
        DWORD thread_id{};
        // Создаем поток обслуживающего сервера.
        rc = CreateThread( NULL, NULL,
            (LPTHREAD_START_ROUTINE)TABLESERVICE_FN( i ),
            (LPVOID)&item,
            NULL, &thread_id );
        // Записываем id потока, в котором открыт обслуживающий сервер для клиента.
        ( *(Contact*)&item ).thread_id = thread_id;
        // Записываем handle потока, обслуживающего сервера.
        ( *(Contact*)&item ).hthread = rc;
    }

    // Возвращаем HANDLE созданного потока.
    return rc;
};

BOOL APIENTRY DllMain( HANDLE hinst, DWORD rcall, LPVOID wres )
{
    return TRUE;
}
