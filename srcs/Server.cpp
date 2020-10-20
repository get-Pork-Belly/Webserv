#include "Server.hpp"
#include "utils.hpp"
#include "ServerManager.hpp"

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

void
Server::init()
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
    fcntl(this->_server_socket, F_SETFL, O_NONBLOCK);
    std::cout<<"server socket fd: " << this->_server_socket <<std::endl;
    int option = true;
    setsockopt(_server_socket, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(int));

    memset(&this->_server_address, 0, sizeof(this->_server_address));
    this->_server_address.sin_family = AF_INET;
    this->_server_address.sin_addr.s_addr = ft::hToNL(INADDR_ANY);
    this->_server_address.sin_port = ft::hToNS(stoi(this->_port));

    if (bind(this->_server_socket, reinterpret_cast<struct sockaddr *>(&this->_server_address),
        static_cast<socklen_t>(sizeof(this->_server_address))))
        throw "Bind error";

    if (listen(this->_server_socket, 128) == -1)
        throw "Listen error";
}

Request
Server::receiveRequest(int fd)
{
    Request req;
    int bytes;
    int len;
    std::string req_message;
    char buf[BUFFER_SIZE + 1];

    bytes = 0;
    memset(reinterpret_cast<void *>(buf), 0, BUFFER_SIZE + 1);

    while ((len = recv(fd, buf, BUFFER_SIZE, MSG_PEEK)) > 0)
    {
        bytes = read(fd, buf, len);
        buf[bytes] = 0;
        req_message += buf;
    }

    //TODO 우아한 종료 되었을 때 체크하기.
    // if (len == 0)

    //TODO valid 체크 반영하기
    if (bytes >= 0)
        req.parseRequest(req_message);
    else if (bytes < 0)
    {
        req.setStatusCode("400");
        return (req);
    }
    return (req);
}

//TODO 이 부분 작업하기.
void
Server::makeResponse(Request& request, int fd)
{
    (void)fd;
    (void)request;
    std::string res = "OPTION / HTTP/1.1\r\nHost: 127.0.0.1:8080\r\nUser-Agent: Mozilla/5.0 (Macintosh;) Firefox/51.0\r\nAccept-Charsets: text/html\r\nAccept-Language: en-US\r\nContent-Type: multipart/form-data\r\nContent-Length: 345\r\n\r\nHelloWorld, everybody, buddy, whatsup\r\n";
}

bool
Server::sendResponse(int fd)
{
    std::cout<<"in sendResponse fd: " << fd << std::endl;
    return (true);
}

bool
Server::isClientOfServer(int fd)
{
    return ((std::find(this->_client_sockets.begin(),
            this->_client_sockets.end(), fd)
            == this->_client_sockets.end()) ? false : true);
}

void
Server::run(ServerManager *server_manager, int fd)
{
    int client_len;
    int client_socket;
    struct sockaddr_in client_address;

    if (fd == this->getServerSocket())
    {
        client_len = sizeof(client_address);
        if ((client_socket = accept(this->getServerSocket(), reinterpret_cast<struct sockaddr *>(&client_address), reinterpret_cast<socklen_t *>(&client_len))) == -1)
            std::cerr<<"accept error"<<std::endl;
        this->_client_sockets.push_back(client_socket);
        if (client_socket > server_manager->getFdMax())
            server_manager->setFdMax(client_socket);
        server_manager->fdSet(client_socket, READ_FDSET);
        fcntl(client_socket, F_SETFL, O_NONBLOCK);
    }
    else
    {
        if (server_manager->fdIsSet(fd, WRITE_FDSET))
        {
            if (!(sendResponse(fd)))
                std::cerr<<"Error: sendResponse"<<std::endl;
            server_manager->fdClr(fd, WRITE_FDSET);
            close(fd);
            _client_sockets.erase(std::find(_client_sockets.begin(),
                        _client_sockets.end(), fd));
            //TODO setFdMax를 효율적으로 할것.
            if (fd == server_manager->getFdMax())
                server_manager->setFdMax(fd - 1);
        }
        else if (server_manager->fdIsSet(fd, READ_FDSET))
        {
            Request request(this->receiveRequest(fd));
            this->makeResponse(request, fd);
            server_manager->fdSet(fd, WRITE_FDSET);
            server_manager->fdClr(fd, READ_FDSET);
        }
    }
}
