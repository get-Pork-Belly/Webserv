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

// const char*
// Server::ParseRequestException::what() const throw()
// {
//     return ("ParseRequestException: Failed request parsing.");
// }

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

void
Server::receiveRequest(int fd)
{
    int bytes;
    int len;
    char buf[BUFFER_SIZE + 1];
    std::string req_message;
    Request& req = this->_requests[fd];
    ServerManager* server_manager = this->_server_manager;
    size_t header_end_pos = 0;

std::cout << "TYPE" << static_cast<int>(req.getReqInfo()) << std::endl;

    bytes = -42;
    ft::memset(reinterpret_cast<void *>(buf), 0, BUFFER_SIZE + 1);

    // header나 body의 CRLF 연속으로 있는 부분 인덱스 찾아야함.
    // req.clear() 는 COMPLETE일 때 리스폰스를 만들고 없애자.
    if (req.getReqInfo() == ReqInfo::READY)
    {
        if ((len = recv(fd, buf, BUFFER_SIZE, MSG_PEEK)) > 0)
        {
            if ((header_end_pos = std::string(buf).find("\r\n\r\n")) != std::string::npos)
            {
                if ((bytes = read(fd, buf, header_end_pos + 4)) < 0)
                {
                    req.setStatusCode("400");
                    throw "";
                    // throw (SocketReadException());
                }
                else if (bytes == 0)
                {
                    req.setStatusCode("502"); // "Bad GateWay"
                    throw "";
                    // throw (BadGateWayException());
                }
                else
                {
                    req_message += buf;
                    req.parseRequestWithoutBody(req_message);
                    req.updateReqInfo();
                    if (req.getReqInfo() == ReqInfo::COMPLETE)
                    {
                        server_manager->fdSet(fd, FdSet::WRITE);
                        this->_server_manager->fdClr(fd, FdSet::READ);
                    }
                }
            }
            else
            {
                // throw (RequestFormatException());
                req.setStatusCode("400");
                return ;
            }
        }
        else if (len == 0)
        {
            this->closeClientSocket(fd);
        }
        else
        {
            req.setStatusCode("400");
            Log::closeClient(*this, fd);
            // throw (RecvErrorException());
        }
    }
    else if (req.getReqInfo() == ReqInfo::NORMAL_BODY)
    {
        //TODO: 쓰레기값 들어옴 같은 fd에 대해 버퍼 클리어해주기.

        int content_length;
        std::map<std::string, std::string> headers;
        headers = req.getHeaders();
        location_info::iterator it;
        it = headers.find("Content-Length");
        if (it == req.getHeaders().end())
            throw "";
            // throw (NoContentLengthException());
        else
            content_length = std::stoi(it->second);

        const int size = (BUFFER_SIZE < content_length) ? content_length : BUFFER_SIZE;

        //TODO: 만약 일정 크기 이상의 바디가 들어오면 청크드로 다시 보내라는 등 메세지 띄워도 좋을듯
        if (size > 50000)
            throw "";

        //TODO: 동적할당 고려해보기
        char body_buf[size + 1];
        ft::memset(reinterpret_cast<void *>(body_buf), 0, size + 1);


        if ((bytes = recv(fd, body_buf, size, 0)) < 0)
        {
             req.setStatusCode("400");
             throw "";
             // throw (SocketReadException());
        }
        else if (bytes == 0)
            this->closeClientSocket(fd);
        else
        {
            req_message += body_buf;
            req.parseBodies(req_message);
            this->_server_manager->fdSet(fd, FdSet::WRITE);
        }
    }
    else if (req.getReqInfo() == ReqInfo::CHUNKED_BODY)
    {
        if ((bytes = recv(fd, buf, BUFFER_SIZE, 0)) < 0)
        {
            req.setStatusCode("400");
            throw "";
        }
        else if (bytes == 0)
            this->closeClientSocket(fd);
        else
        {
            req_message += buf;
            if (req.parseChunkedBody(req_message) && req.getReqInfo() == ReqInfo::COMPLETE)
                this->_server_manager->fdSet(fd, FdSet::WRITE);
            else
            {
                req.setStatusCode("400");
                throw "";
            }
        }
    }
    else if (req.getReqInfo() == ReqInfo::COMPLETE)
    {
        std::cout << "In Complete" << std::endl;
        std::string method = req.getMethod();
        this->_server_manager->fdClr(fd, FdSet::READ);
        // if (!(method.compare("PUT") || method.compare("POST")))
        // {
        //     // recv(); 버퍼를 비우는 용도로~
        // }
    }
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
    //     headers += ("\n" + yellow + "value: " + reset + m.second +j "\n");
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
            try
            {
                // isResource, isCGIPipe, isClient
                if (this->isCGIPipe(fd))
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
                else if (this->isStaticResource(fd))
                {
                }
                else if (this->isClientSocket(fd))
                {
                    this->receiveRequest(fd);
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
            catch(const std::exception& e)
            {
                // this->_server_manager->fdClr(fd, FdSet::READ);
                this->closeClientSocket(fd);
                std::cerr << e.what() << '\n';
            }
 
        }
    }
}

bool
Server::closeClientSocket(int fd)
{
    int ret;
    Log::closeClient(*this, fd);

    std::cout << "In Close" << std::endl;

    this->_server_manager->fdClr(fd, FdSet::READ);
    this->_server_manager->setClosedFdOnFdTable(fd);
    this->_server_manager->updateFdMax(fd);
    this->_requests[fd].clear();
    Log::closeClient(*this, fd);
    if ((ret = close(fd)) < 0)
        return (false);
    return (true);
}