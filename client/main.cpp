#include "client.h"

int main( int argc, char** argv )
{
    if ( argc < 2 )
    {
        std::cout << "[ ERROR ] Too less args. First argument must be a port." << std::endl;
        std::cin.get();
        return EXIT_FAILURE;
    }

    Client client( std::stoi( argv[1] ) );
    client.find_server();
    client.connect_to_service_server();

    std::cout << "Enter any key to exit..." << std::endl;
    std::cin.get();
    return EXIT_SUCCESS;
}
