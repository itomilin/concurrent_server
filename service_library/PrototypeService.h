#pragma once

#include <Windows.h>
#include "../defs/defs.h"

//DWORD( ( WINAPI* EchoServer ) ) ( LPVOID );
//DWORD( ( WINAPI* TimeServer ) ) ( LPVOID );
//DWORD( ( WINAPI* ServiceServer01 ) ) ( LPVOID );

DWORD WINAPI EchoServer( LPVOID data );
DWORD WINAPI TimeServer( LPVOID data );
DWORD WINAPI ServiceServer( LPVOID data );
