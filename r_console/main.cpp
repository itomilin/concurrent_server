#include <iostream>
#include <string>

#include <WinSock2.h>
#include <Windows.h>

#include "../defs/defs.h"

int main()
{
    HANDLE hPipe; // дескриптор канала

    if ( ( hPipe = CreateFile(
        L"\\\\.\\pipe\\ConsolePipe",
        GENERIC_READ | GENERIC_WRITE,
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        NULL, OPEN_EXISTING, NULL,
        NULL ) ) == INVALID_HANDLE_VALUE )
    {
        std::cout << "[ ERROR ] Pipe. Create file: "
            << GetLastError() << std::endl;

        std::cout << "Press any key to exit..." << std::endl;
        std::cin.get();
        exit( EXIT_FAILURE );
    }

    // Отправляем сообщения по каналу.
  //  while ( true )
    {
        char msg[] {"\0"};
        LPDWORD countReadedBytes{};

        std::cout << "Input a command: ";
        std::cin.getline( msg, sizeof( msg ) );

        auto rc = WriteFile( hPipe, msg, sizeof( msg ), countReadedBytes, NULL );

     /*   std::string answer {};
        std::cout << "Input \"yes\" to exit: ";
        std::getline( std::cin, answer );

        if ( answer == "yes" )
            break;*/
    }

    // Закрываем дескриптор.
    CloseHandle( hPipe );

    std::cin.get();
    return EXIT_SUCCESS;
}
