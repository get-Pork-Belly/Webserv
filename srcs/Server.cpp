#include "Server.hpp"
#include "utils.hpp"
#include "ServerManager.hpp"
#include "Log.hpp"

/*============================================================================*/
/****************************  Static variables  ******************************/
/*============================================================================*/

/*============================================================================*/
/******************************  Constructor  *********************************/
/*============================================================================*/

Server::Server(ServerManager* server_manager, server_info& server_config, std::map<std::string, location_info>& location_config)
: _server_manager(server_manager), _server_config(server_config),
_server_socket(-1), _server_name(""), _host(""), _port(""),
_status_code(0), _request_uri_limit_size(0), _request_header_limit_size(0), 
_limit_client_body_size(0), _default_error_page(""), 
_location_config(location_config)
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

const std::map<std::string, std::string> 
Server::getServerConfig()
{
    return (this->_server_config);
}

int
Server::getServerSocket() const
{
    return (this->_server_socket);
}

const std::map<std::string, location_info>&
Server::getLocationConfig()
{
    return (this->_location_config);
}

Request
Server::getRequest(int fd)
{
    return (this->_requests[fd]);
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
        // if (conf.first == "server_name")
        //     this->_server_name = conf.second;
        if (conf.first == "host")
            this->_host = conf.second;
        else if (conf.first == "listen")
            this->_port = conf.second;
    }
    this->_requests = std::vector<Request>(1024);

    if ((this->_server_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1)
        throw "Socket Error";
    fcntl(this->_server_socket, F_SETFL, O_NONBLOCK);
    Log::serverIsCreated(*this);
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

    this->_server_manager->setServerSocketOnFdTable(this->_server_socket);
    this->_server_manager->updateFdMax(this->_server_socket);
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

    // TODO: receive의 구조 바꾸기
    // 처음에는 Header까지만 읽은 다음에 URI, Chunked 등의 미리 알아둬야 할 정보가 있다면 세팅을 하고
    // 나머지 데이터를 읽는 방향으로 수정하자.

    if ((len = recv(fd, buf, BUFFER_SIZE, MSG_PEEK)) > 0)
    {
        if ((bytes = read(fd, buf, BUFFER_SIZE)) < 0)
        {
            req.setStatusCode("400");
            return (req);
        }
        buf[bytes] = 0;
        req_message += buf;
        server_manager->fdSet(fd, FdSet::WRITE);
        req.parseRequest(req_message);
    }
    else if (len == 0)
    {
        std::cout<<"len: 0"<<std::endl;
        server_manager->fdClr(fd, FdSet::READ);
        close(fd);
        this->_server_manager->setClosedFdOnFdTable(fd);
        this->_server_manager->updateFdMax(fd);
        Log::closeClient(*this, fd);
    }
    else
    {
        std::cout<<"len: -1"<<std::endl;
        req.setStatusCode("400");
        server_manager->fdClr(fd, FdSet::READ);
        close(fd);
        this->_server_manager->setClosedFdOnFdTable(fd);
        this->_server_manager->updateFdMax(fd);
        Log::closeClient(*this, fd);
    }
    // if (bytes >= 0)
    //     req.parseRequest(req_message);
    return (req);
}

std::string
Server::makeResponseMessage(Request& request)
{
    Response response;
    std::string status_line;
    std::string headers;
    std::string body;

    response.applyAndCheckRequest(request, this);
    // body = response.makeBody(request);
    // headers = response.makeHeaders(request);
    status_line = response.makeStatusLine();
    return (status_line + headers + body);
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
Server::isFdManagedByServer(int fd) const
{
    const std::vector<std::pair<FdType, int> >& fd_table = this->_server_manager->getFdTable();

    if (fd_table[fd].first == FdType::CLIENT_SOCKET)
        return (isClientOfServer(fd));
    else if (fd_table[fd].first == FdType::RESOURCE || fd_table[fd].first == FdType::PIPE)
        return (isClientOfServer(fd_table[fd].second));
    return (false);
}

bool
Server::isClientOfServer(int fd) const
{
    const std::vector<std::pair<FdType, int> >& fd_table = this->_server_manager->getFdTable();
    return (fd_table[fd].first == FdType::CLIENT_SOCKET && fd_table[fd].second == this->getServerSocket());
}

bool
Server::isServerSocket(int fd) const
{
    const std::vector<std::pair<FdType, int> >& fd_table = this->_server_manager->getFdTable();
    if (fd_table[fd].first == FdType::SERVER_SOCKET)
        return true;
    return false;
}

bool
Server::isClientSocket(int fd) const
{
    const std::vector<std::pair<FdType, int> >& fd_table = this->_server_manager->getFdTable();
    if (fd_table[fd].first == FdType::CLIENT_SOCKET)
        return true;
    return false;
}

bool
Server::isStaticResource(int fd) const
{
    const std::vector<std::pair<FdType, int> >& fd_table = this->_server_manager->getFdTable();
    if (fd_table[fd].first == FdType::RESOURCE)
        return true;
    return false;
}

bool
Server::isCGIPipe(int fd) const
{
    const std::vector<std::pair<FdType, int> >& fd_table = this->_server_manager->getFdTable();
    if (fd_table[fd].first == FdType::PIPE)
        return true;
    return false;
} 

void
Server::run(int fd)
{
    int client_len;
    int client_socket;
    struct sockaddr_in client_address;

    if (isServerSocket(fd))
    {
        client_len = sizeof(client_address);
        if ((client_socket = accept(this->getServerSocket(),
            reinterpret_cast<struct sockaddr *>(&client_address),
            reinterpret_cast<socklen_t *>(&client_len))) != -1)
        {
            if (client_socket > this->_server_manager->getFdMax())
                this->_server_manager->setFdMax(client_socket);
            this->_server_manager->fdSet(client_socket, FdSet::READ);
            fcntl(client_socket, F_SETFL, O_NONBLOCK);
            this->_server_manager->setClientSocketOnFdTable(client_socket, this->getServerSocket());
            this->_server_manager->updateFdMax(client_socket);
            Log::newClient(*this, client_socket);
        }
        else
            std::cerr<<"Accept error"<<std::endl;
    }
    else
    {
        if (this->_server_manager->fdIsSet(fd, FdSet::WRITE))
        {
            // if (isClientSocket(fd))
            // {
                std::string response_message;
                response_message = this->makeResponseMessage(this->_requests[fd]);
                // TODO: sendResponse error handling
                if (!(sendResponse(response_message, fd)))
                    std::cerr<<"Error: sendResponse"<<std::endl;
                this->_server_manager->fdClr(fd, FdSet::WRITE);
            // }
        }
        else if (this->_server_manager->fdIsSet(fd, FdSet::READ))
        {
            // isResource, isCGIPipe, isClient
            if (this->isStaticResource(fd))
            {
                // char test_buf[4096];
                // ft::memset(static_cast<void *>(test_buf), 0, sizeof(test_buf));
                // read(fd, test_buf, 4096);
                // std::cout<<"========== test  buf ========="<<std::endl;
                // std::cout<<test_buf<<std::endl;
                // // std::cout<<"TEST Success!!"<<std::endl;
                // close(fd);
                // this->_server_manager->setClosedFdOnFdTable(fd);
                // this->_server_manager->updateFdMax(fd);
                // this->_server_manager->fdClr(fd, FdSet::READ);
            }
            else if (this->isCGIPipe(fd))
            {
            }
            else if (this->isClientSocket(fd))
            {
                Request request(this->receiveRequest(this->_server_manager, fd));
                _requests[fd] = request;
                // 
                // Request의 body 이전까지만 읽는다. 그리고 URI를 체크한다. 체크는 메서드 체크를 1차적으로 하고, URI를 체크한다.
                // 만약 요청이 static_resource를 요구했다면, 
                // fd순회를 반복하여 Request를 마저 읽는다. 
                // static_resource를 open한 다음, read_fdset에 등록하고 fd 순회를 계속한다.
                // 만약 요청이 CGI를 요구했다면(판단은 URI의 확장자로!)
                // fd순회를 반복하여 Request를 마저 읽는다. 
                // 다 읽으면 read_CLR(fd)하고, pipe를 설치한 다음에, read_fdset에 등록하고 fd 순회를 계속한다.
                // 만약 요청에 대해서 별도의 body를 구성할 필요가 없다면, 걍 Response를 완성시킨 다음 write_fdset에 client_socket을 등록한다.


                // int tmp_fd = open("Makefile", O_RDONLY, 0644);
                // this->_server_manager->setResourceOnFdTable(tmp_fd, fd);
                // this->_server_manager->updateFdMax(tmp_fd);
                // this->_server_manager->fdSet(tmp_fd, FdSet::READ);
                // std::cout<<"tmp_fd: "<<tmp_fd<<std::endl;
                // std::cout<<"Max fd: "<<this->_server_manager->getFdMax()<<std::endl;
                Log::getRequest(*this, fd);
            }
        }
    }
}
