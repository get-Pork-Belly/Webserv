#include "Server.hpp"

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
    //NOTE: init 시 bind, listen까지 이루어짐.
    if (!Server::init())
        throw "Server init error";
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

bool Server::init()
{

    //TODO: 0 등으로 초기화했던 private 멤버들의 값을 세팅해주어야 한다.
    // this->_server_socket =
    // this->_client_sockets =
    for (auto& conf: this->_server_config)
    {
        if (conf.first == "server_name")
            this->_server_name = conf.second;
        else if (conf.first == "host")
            this->_host = conf.second;
        else if (conf.first == "port")
            this->_port = conf.second;
    }

    //NOTE: Test용 cout
    std::cout << this->_server_name << std::endl;
    std::cout << this->_host << std::endl;
    std::cout << this->_port << std::endl;

    // this->_status_code =
    // this->_request_uri_limit_size = 
    // this->_request_header_limit_size =
    // this->_limit_client_body_size =
    // this->_default_error_page =


    if ((this->_server_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1)
    {
        throw "Socket Error";
        return false;
    }

    //NOTE: Test용 cout
    std::cout << this->_server_socket << std::endl;

    int option = true;
    setsockopt(_server_socket, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(int));

    //TODO: memset 구현하기
    memset(&this->_server_address, 0, sizeof(this->_server_address));
    this->_server_address.sin_family = AF_INET;;
    //TODO: htonl, htons 구현
    //NOTE: stoi 사용가능한지 확인하기
    this->_server_address.sin_addr.s_addr = htonl(INADDR_ANY);
    this->_server_address.sin_port = htons(stoi(this->_port));

    if (bind(this->_server_socket, reinterpret_cast<struct sockaddr *>(&this->_server_address), static_cast<socklen_t>(sizeof(this->_server_address))))
    {
        throw "Bind error";
        return false;
    }

    if (listen(this->_server_socket, 128) == -1)
    {
        throw "Listen error";
        return false;
    }

    return true;
}
