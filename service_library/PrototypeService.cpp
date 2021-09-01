#include "pch.h"

#include "PrototypeService.h"

DWORD EchoServer( LPVOID data )
{
    auto test = (Contact*)data;
    auto xxx = "sdf";

    ExitThread( *(DWORD*)data );
}

DWORD TimeServer( LPVOID data )
{
    ExitThread( *(DWORD*)data );
}

DWORD ServiceServer( LPVOID data )
{
    ExitThread( *(DWORD*)data );
}