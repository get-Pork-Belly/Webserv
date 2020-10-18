#include "ServerManager.hpp"
#include <iostream>

int main(int argc, char *argv[], char *envp[])
{
    (void)envp;
    if (argc > 2)
    {
        //TODO: error message
        return (EXIT_FAILURE);
    }

    const char *default_path = "/default.conf/";
    const char *config_path = (argc == 1) ? default_path : argv[1];
    try
    {
        ServerManager server_manager(config_path);
        // server_manager 객체가 생성되며, 생성자안에서 serverInit 함수가 실행된다.
        // server_manager.init();
        if (!server_manager.runServers())
        {
            std::cerr<<"error"<<std::endl;
            // server_manager.exitServers();
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