#include "Server.hpp"
#include "utils.hpp"

//NOTE: 테스트용 iostream 헤더
#include <iostream>


/*============================================================================*/
/****************************  Static variables  ******************************/
/*============================================================================*/

/*============================================================================*/
/******************************  Constructor  *********************************/
/*============================================================================*/

Server::Server(std::map<std::string, std::string>& server_config)
: _server_config(server_config), _server_socket(-1), _client_sockets(0), _server_name(""), _host(""), _port(""), _status_code(0), _request_uri_limit_size(0), _request_header_limit_size(0), _limit_client_body_size(0), _default_error_page("")
{
    try
    {
        this->init();
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
        throw "Server init error";
    }
}

/*============================================================================*/
/******************************  Destructor  **********************************/
/*============================================================================*/

Server::~Server()
{
}

/*============================================================================*/
/*******************************  Overload  ***********************************/
/*============================================================================*/

/*============================================================================*/
/********************************  Getter  ************************************/
/*============================================================================*/

const std::map<std::string, std::string> Server::getServerConfig()
{
    return (this->_server_config);
}

int Server::getServerSocket()
{
    return (this->_server_socket);
}

/*============================================================================*/
/********************************  Setter  ************************************/
/*============================================================================*/

// void setServerSocket(int server_socket)
// {
//     this->_server_socket = server_socket;
// }

/*============================================================================*/
/******************************  Exception  ***********************************/
/*============================================================================*/

/*============================================================================*/
/*********************************  Util  *************************************/
/*============================================================================*/

void Server::init()
{

    for (auto& conf: this->_server_config)
    {
        if (conf.first == "server_name")
            this->_server_name = conf.second;
        else if (conf.first == "host")
            this->_host = conf.second;
        else if (conf.first == "port")
            this->_port = conf.second;
    }

    if ((this->_server_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1)
        throw "Socket Error";

    int option = true;
    setsockopt(_server_socket, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(int));

    //TODO: memset 구현하기
    memset(&this->_server_address, 0, sizeof(this->_server_address));
    this->_server_address.sin_family = AF_INET;;
    //NOTE: stoi 사용가능한지 확인하기
    this->_server_address.sin_addr.s_addr = ft::hToNL(INADDR_ANY);
    this->_server_address.sin_port = ft::hToNS(stoi(this->_port));

    if (bind(this->_server_socket, reinterpret_cast<struct sockaddr *>(&this->_server_address), static_cast<socklen_t>(sizeof(this->_server_address))))
        throw "Bind error";

    if (listen(this->_server_socket, 128) == -1)
        throw "Listen error";
}
