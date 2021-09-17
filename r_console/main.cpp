#include <iostream>
#include <string>
#include <limits>
#include <cstring>

#include <WinSock2.h>
#include <Windows.h>

#include "../defs/defs.h"

#include <sstream>


//#pragma warning (disable : 4996)

int main( int argc, char** argv )
{
    if ( argc < 2 )
    {
        std::wcout << "[ ERROR ] Check args." << std::endl;
    }

    HANDLE hPipe{}; // дескриптор канала

    // Перевод параметра в LPCWSTR.
    std::string name = argv[1];
    std::wstring wide = L"\\\\.\\pipe\\";

    std::wstringstream cls;
    cls << wide.c_str() << name.c_str();
    std::wstring total = cls.str();

    if ( ( hPipe = CreateFile(
        total.c_str(),
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
    while ( true )
    {
        int16_t input {};
        std::cout << "0. START" << std::endl
            << "1. STOP" << std::endl
            << "2. EXIT" << std::endl
            << "3. STATISTICS" << std::endl
            << "4. WAIT" << std::endl
            << "5. SHUTDOWN" << std::endl
            << "Select a command to server: ";
        
        // Проверка на то, что ввели 0-9.
        if ( !( std::cin >> input ) )
        {
            std::cin.clear();
            std::cin.ignore( 123, '\n' );
            std::cout << "Input error." << std::endl;
            continue;
        }

        char buf[256]{ };
        char out_msg[256]{ };
        LPDWORD countReadedBytes{};
        LPDWORD countReadedBytes2{};

        std::sprintf( buf, "%i", input );
        auto rc = WriteFile( hPipe, buf, sizeof( buf ), countReadedBytes, NULL );

        auto answer = ReadFile( hPipe, out_msg, sizeof( out_msg ), countReadedBytes2, NULL );
        std::cout << "Answer is: " << out_msg << std::endl << std::endl;

        if ( input == 2 || input == 5 )
            break;
    }

    // Закрываем дескриптор.
    CloseHandle( hPipe );
    std::cout << "Server is shutdown. Press any key to exit..." << std::endl;
    std::cin.ignore();
    std::cin.get();
    return EXIT_SUCCESS;
}
