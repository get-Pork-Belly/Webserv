#include "PageGenerator.hpp"
#include "Log.hpp"
#include <iostream>
#include "ServerManager.hpp"
#include "Server.hpp"
#include "Request.hpp"
#include "Response.hpp"

int main(int argc, char *argv[])
{
    if (argc > 2)
    {
        //TODO: error message
        return (EXIT_FAILURE);
    }

    const char *default_path = "tests/yohlee_config";
    const char *config_path = (argc == 1) ? default_path : argv[1];
    try
    {
        ServerManager server_manager(config_path);
        if (!server_manager.runServers())
        {
            std::cerr<<"error"<<std::endl;
        }
        else
        {
            std::cout<<"success run server"<<std::endl;
        }
        
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
    }
    catch(const char *e)
    {
        std::cerr<<e<<std::endl;
    }
 
    return (0);
}
