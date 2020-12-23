#include "Server.hpp"
#include <iostream>
#include <exception>

int main(void)
{
    try
    {
        std::map<std::string, std::string> server_config = {{"server_name", "sanam"}, {"host", "127.0.0.1"}, {"port", "8080"}};
        Server server(server_config);

        std::cout<<"Server constructor success"<<std::endl;

        // const std::map<std::string, std::string> m = server.getServerConfig();
        // for (auto& kv: server.getServerConfig())
        // {
        //     std::cout<< kv.first<<": "<<kv.second<<std::endl;
        // }
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
    }
    

    return (0);
}