#include "Server.hpp"
#include "utils.hpp"
#include "ServerManager.hpp"

//NOTE: 테스트용 iostream 헤더
#include <iostream>


/*============================================================================*/
/****************************  Static variables  ******************************/
/*============================================================================*/

/*============================================================================*/
/******************************  Constructor  *********************************/
/*============================================================================*/

Server::Server(server_info& server_config, std::map<std::string, location_info> location_config)
: _server_config(server_config), _server_socket(-1),
_client_sockets(0), _server_name(""), _host(""),
_port(""), _status_code(0), _request_uri_limit_size(0),
_request_header_limit_size(0), _limit_client_body_size(0), _default_error_page("")
{
    //TODO: location_config 를 서버에 반영
    (void)location_config;
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
        else if (conf.first == "listen")
            this->_port = conf.second;
    }

    if ((this->_server_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1)
        throw "Socket Error";

    int option = true;
    setsockopt(_server_socket, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(int));

    //TODO: memset 구현하기
    memset(&this->_server_address, 0, sizeof(this->_server_address));
    this->_server_address.sin_family = AF_INET;;
    this->_server_address.sin_addr.s_addr = ft::hToNL(INADDR_ANY);
    this->_server_address.sin_port = ft::hToNS(stoi(this->_port));

    if (bind(this->_server_socket, reinterpret_cast<struct sockaddr *>(&this->_server_address), static_cast<socklen_t>(sizeof(this->_server_address))))
        throw "Bind error";

    if (listen(this->_server_socket, 128) == -1)
        throw "Listen error";
}

void Server::run(ServerManager *server_manager)
{
    int client_len;
    struct sockaddr_in client_address;
    int client_socket;

    for (int fd = 0; fd < server_manager->getFdMax(); fd++)
    {
        if (server_manager->fdIsSet(fd, ALL_FDSET))
        {
            if (fd == this->getServerSocket())
            {
                client_len = sizeof(client_address);
                //TODO: client_address 지역변수로 써도되는지 체크
                if ((client_socket = accept(this->getServerSocket(), reinterpret_cast<struct socketaddr *>(&client_address), reinterpret_cast<socklen_t *>(&client_len))) == -1)
                    std::cerr<<"accept error"<<std::endl;
                if (server_manager->getFdMax() < client_socket)
                    server_manager->setFdMax(client_socket);
            }
            else
            {
                if (server_manager->fdIsSet(fd, WRITE_FDSET))
                {
                    if (!(sendResponse(fd))
                    {
                        std::cerr<<"Error: sendResponse"<<std::endl;
                    }
                    server_manager->fdClr(fd, WRITE_FDSET);
                    continue ;
                }
                if (server_manager->fdIsSet(fd, READ_FDSET))
                {
                    this->makeResponse(this->receiveRequest(), fd);
                    server_manager->fdSet(fd, WRITE_FDSET);
                    server_manager->fdClr(fd, READ_FDSET);
                }
            }
        }
    }
}