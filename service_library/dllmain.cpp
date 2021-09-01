﻿// dllmain.cpp : Defines the entry point for the DLL application.
#include "pch.h"

#include "DefineTableService.h" // макро для TableService
#include "PrototypeService.h" // прототипы обслуживающих потоков

BEGIN_TABLESERVICE
ENTRYSERVICE( "echo", EchoServer ),
ENTRYSERVICE( "Time", TimeServer ),
ENTRYSERVICE( "rand", ServiceServer )
END_TABLESERVICE;

extern "C" __declspec( dllexport ) HANDLE SSS( char* id, LPVOID prm )
{
    HANDLE rc = NULL;
    int i = 0;
    auto test = SIZETS;
    auto test2 = TABLESERVICE_ID( i );

    while ( i < SIZETS && strcmp( TABLESERVICE_ID( i ), id ) != 0 )
        i++;

    if ( i < SIZETS )
    {
        rc = CreateThread( NULL, NULL,
            TABLESERVICE_FN( i ),
            prm,
            NULL, NULL );
    }

    return rc;
};

BOOL APIENTRY DllMain( HANDLE hinst, DWORD rcall, LPVOID wres )
{
    return TRUE;
}
