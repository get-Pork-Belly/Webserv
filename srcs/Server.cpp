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
    this->_requests = std::vector<Request>(1024);

    if ((this->_server_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1)
        throw "Socket Error";
    fcntl(this->_server_socket, F_SETFL, O_NONBLOCK);
    std::cout<<"server socket fd: " << this->_server_socket <<std::endl;
    int option = true;
    setsockopt(_server_socket, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(int));

    ft::memset(&this->_server_address, 0, sizeof(this->_server_address));
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
Server::receiveRequest(ServerManager* server_manager, int fd)
{
    Request req;
    int bytes;
    int len;
    std::string req_message;
    char buf[BUFFER_SIZE + 1];

    bytes = -42;
    memset(reinterpret_cast<void *>(buf), 0, BUFFER_SIZE + 1);

    if ((len = recv(fd, buf, BUFFER_SIZE, MSG_PEEK)) > 0)
    {
        if ((bytes = read(fd, buf, BUFFER_SIZE)) < 0)
        {
            req.setStatusCode("400");
            return (req);
        }
        buf[bytes] = 0;
        req_message += buf;
        server_manager->fdSet(fd, WRITE_FDSET);
        req.parseRequest(req_message);
    }
    else if (len == 0)
    {
        std::cout<<"len: 0"<<std::endl;
        server_manager->fdClr(fd, READ_FDSET);
        close(fd);
        _client_sockets.erase(std::find(_client_sockets.begin(), _client_sockets.end(), fd));
        //TODO setFdMax를 효율적으로 할것.
        if (fd == server_manager->getFdMax())
            server_manager->setFdMax(fd - 1);
    }
    else
    {
        std::cout<<"len: -1"<<std::endl;
        req.setStatusCode("400");
        server_manager->fdClr(fd, READ_FDSET);
        close(fd);
        _client_sockets.erase(std::find(_client_sockets.begin(), _client_sockets.end(), fd));
        //TODO setFdMax를 효율적으로 할것.
        if (fd == server_manager->getFdMax())
            server_manager->setFdMax(fd - 1);
    }
    // if (bytes >= 0)
    //     req.parseRequest(req_message);
    return (req);
}

std::string
Server::makeResponseMessage(Request& request)
{
    Response response;
    std::string start_line;
    std::string headers;
    std::string body;

    (void)request;
    // body = response.makeBody(request);
    // headers = response.makeHeaders(request);
    start_line = response.makeStartLine();
    return (start_line + headers + body);
    // std::string ret;
    // std::string status_line =  "\033[1;31;40mStatus Line\033[0m\n" + request.getRequestMethod() + " " + request.getRequestUri() + request.getRequestVersion();
    // ret = (status_line + "\n");
    // std::cout << "\033[1;31;40mHEADERS\033[0m" << std::endl;
    // std::string blue =  "\033[1;34;40m";
    // std::string yellow =  "\033[1;33;40m";
    // std::string reset = "\033[0m";
    // std::string headers;
    // for (auto& m : request.getRequestHeaders())
    // {
    //     headers += (blue + "key: " + reset + m.first );
    //     headers += ("\n" + yellow + "value: " + reset + m.second + "\n");
    // }
    // ret += headers;
    // std::string response_body = "\n\033[1;34;40mBody\033[0m\n" + request.getRequestBodies() + "\n";
    // ret += response_body;
    // return ret;
}

bool
Server::sendResponse(std::string& response_message, int fd)
{
    std::string tmp = "fd: ";
    tmp += std::to_string(fd);
    tmp += " in send response\n";
    tmp += "===============================\n";
    tmp += "response_message\n ";
    tmp += "===============================\n";
    tmp += response_message;
    write(fd, tmp.c_str(), tmp.length());
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
        if ((client_socket = accept(this->getServerSocket(),
            reinterpret_cast<struct sockaddr *>(&client_address),
            reinterpret_cast<socklen_t *>(&client_len))) != -1)
        {
            this->_client_sockets.push_back(client_socket);
            if (client_socket > server_manager->getFdMax())
                server_manager->setFdMax(client_socket);
            server_manager->fdSet(client_socket, READ_FDSET);
            fcntl(client_socket, F_SETFL, O_NONBLOCK);
        }
        else
            std::cerr<<"Accept error"<<std::endl;
    }
    else
    {
        if (server_manager->fdIsSet(fd, WRITE_FDSET))
        {
            std::string response_message;
            response_message = this->makeResponseMessage(this->_requests[fd]);
            // TODO: sendResponse error handling
            if (!(sendResponse(response_message, fd)))
                std::cerr<<"Error: sendResponse"<<std::endl;
            server_manager->fdClr(fd, WRITE_FDSET);
        }
        else if (server_manager->fdIsSet(fd, READ_FDSET))
        {
            Request request(this->receiveRequest(server_manager, fd));
            _requests[fd] = request;
        }
    }
}
