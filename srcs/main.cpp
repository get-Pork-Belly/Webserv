#include "ServerManager.hpp"
#include <iostream>

// class ServerManager
// {
//     private:
//         ServerManager(); //NOTE 구현x
//         std::string _config_file_path;
//     public:
//         ServerManager(const char *config_path);
//         void initServers();
//         void makeServer(struct s_config);
//         bool runServers();
//         void exitServers();
// };

// ServerManager::ServerManager(const char *config_path)
// {
//     this->initServers();
// }

// ServerManager::initServers()
// {
//     this->makeServer();
// }

// bool ServerManager::runServers()
// {
//     while (true)
//     {
//         for (std::vector<Server *>::iterator iter.begin(); iter < iter.end(); ++iter)
//         {
//             iter->getRequest();
//             iter->isValidRequest();
//             iter->makeResponse();
//         }
//     }
// }

int main(int argc, char *argv[], char *envp[])
{

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
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
    }

    if (!server_manager.runServers())
    {
        //TODO: error처리코드
        std::cerr<"error"<<std::endl;
        server_manager.exitServers();
    }
    return (0);
}

