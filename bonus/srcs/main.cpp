#include "PageGenerator.hpp"
#include "Log.hpp"
#include "ServerManager.hpp"
#include "Server.hpp"
#include "Request.hpp"
#include "Response.hpp"
#include <iostream>

int main(int argc, char* argv[])
{
    if (argc > 2)
    {
        std::cerr << "\033[31m\033[01m";
        std::cerr << "===============================================" << std::endl;
        std::cerr << "Please put less than two argv." << std::endl;
        std::cerr << "===============================================" << std::endl;
        std::cerr << "\033[0m";
        return (EXIT_FAILURE);
    }

    const char* default_path = "tests/yohlee_config";
    const char* config_path = (argc == 1) ? default_path : argv[1];
    try
    {
        ServerManager server_manager(config_path);
        while (true)
        {
            if (!server_manager.runServers())
            {
                ServerManager new_server_manager(config_path);
                server_manager = new_server_manager;
            }
        }
    }
    catch(const std::exception& e)
    {
        std::cerr << "The server shut down due to an unexpected situation: ";
        std::cerr << e.what() << std::endl;
    }
    catch(const char* e)
    {
        std::cerr << e << std::endl;
    }
 
    return (0);
}
