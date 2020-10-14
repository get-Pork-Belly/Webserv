#include "Server.hpp"
#include <iostream>

int main(void)
{
    std::map<std::string, std::string> server_config = {{"A", "123"}, {"B", "456"}};
    Server server(server_config);

    std::cout<<"Server constructor success"<<std::endl;
    
    const std::map<std::string, std::string> m = server.getServerConfig();
    for (auto& kv: server.getServerConfig())
    {
        std::cout<<"first: "<<kv.first<<"second: "<<kv.second<<std::endl;
    }

    return (0);
}