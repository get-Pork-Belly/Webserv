#include "Server.hpp"
#include "utils.hpp"
#include "ServerManager.hpp"
#include "Log.hpp"
#include "UriParser.hpp"

/*============================================================================*/
/****************************  Static variables  ******************************/
/*============================================================================*/

/*============================================================================*/
/******************************  Constructor  *********************************/
/*============================================================================*/

Server::Server(ServerManager* server_manager, server_info& server_config, std::map<std::string, location_info>& location_config)
: _server_manager(server_manager), _server_config(server_config),
_server_socket(-1), _server_name(""), _host(""), _port(""),
_request_uri_limit_size(0), _request_header_limit_size(0), 
_limit_client_body_size(BUFFER_SIZE), _default_error_page(""), 
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

Request&
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

Server::PayloadTooLargeException::PayloadTooLargeException(Request& request) 
: _request(request)
{
    this->_request.setStatusCode("413");
}

const char*
Server::PayloadTooLargeException::what() const throw()
{
    return ("[CODE 413] Payload Too Large");
}

const char*
Server::ReadErrorException::what() const throw()
{
    return ("[CODE 900] Read empty buffer or occured reading error");
}

Server::CannotOpenDirectoryException::CannotOpenDirectoryException(Request& req, const std::string& status_code)
: _req(req) 
{
    req.setStatusCode(status_code);
}

std::string
Server::CannotOpenDirectoryException::s_what() const throw()
{
    std::string msg;
    msg += "CannotOpenDirectoryException: ";
    msg += strerror(errno);
    msg += "\n";
    return (msg);
}

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
    this->_responses = std::vector<Response>(1024);

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
Server::readBufferUntilHeaders(int fd, char* buf, size_t header_end_pos)
{
    int bytes;
    Request& req = this->_requests[fd];

    if ((bytes = read(fd, buf, header_end_pos + 4)) > 0)
        req.parseRequestWithoutBody(buf);
    else if (bytes == 0)
        throw (Request::RequestFormatException(req, "400"));
    else
        throw (ReadErrorException());
}

void
Server::receiveRequestWithoutBody(int fd)
{
    int bytes;
    char buf[BUFFER_SIZE + 1];
    size_t header_end_pos = 0;
    Request& req = this->_requests[fd];

    if ((bytes = recv(fd, buf, BUFFER_SIZE, MSG_PEEK)) > 0)
    {
        if ((header_end_pos = std::string(buf).find("\r\n\r\n")) != std::string::npos)
        {
            if (static_cast<size_t>(bytes) == header_end_pos + 4)
                req.setIsBufferLeft(false);
            else
                req.setIsBufferLeft(true);
            this->readBufferUntilHeaders(fd, buf, header_end_pos);
        }
        else
            throw (Request::RequestFormatException(req, "400"));
    }
    else if (bytes == 0)
        this->closeClientSocket(fd);
    else
        throw (ReadErrorException());
}

void
Server::receiveRequestNormalBody(int fd)
{
    int bytes;
    Request& req = this->_requests[fd];

    int content_length = req.getContentLength();
    if (content_length > this->_limit_client_body_size)
        throw (PayloadTooLargeException(req));

    char buf[BUFFER_SIZE + 1];
    ft::memset(reinterpret_cast<void *>(buf), 0, BUFFER_SIZE + 1);

    if ((bytes = recv(fd, buf, content_length, 0)) > 0)
    {
        req.parseNormalBodies(buf);
        this->_server_manager->fdSet(fd, FdSet::WRITE);
    }
    else if (bytes == 0)
        this->closeClientSocket(fd);
    else
        throw (ReadErrorException());
}

void
Server::clearRequestBuffer(int fd)
{
    int bytes;
    char buf[BUFFER_SIZE + 1];
    Request& req = this->_requests[fd];

    if ((bytes = recv(fd, buf, BUFFER_SIZE, 0)) > 0)
    {
        if (bytes == BUFFER_SIZE)
            return ;
        req.setReqInfo(ReqInfo::COMPLETE);
        this->_server_manager->fdSet(fd, FdSet::WRITE);
    }
    else if (bytes == 0)
        this->closeClientSocket(fd);
    else
        throw (ReadErrorException());
}

void
Server::receiveRequestChunkedBody(int fd)
{
    int bytes;
    char buf[BUFFER_SIZE + 1];
    Request& req = this->_requests[fd];

    if ((bytes = recv(fd, buf, BUFFER_SIZE, 0)) > 0)
    {
        req.parseChunkedBody(buf);
        if (req.getReqInfo() == ReqInfo::COMPLETE)
            this->_server_manager->fdSet(fd, FdSet::WRITE);
    }
    else if (bytes == 0)
        this->closeClientSocket(fd);
    else
        throw (ReadErrorException());
}

void
Server::receiveRequest(int fd)
{
    ReqInfo req_info = this->_requests[fd].getReqInfo();

    switch (req_info)
    {
    case ReqInfo::READY:
        this->receiveRequestWithoutBody(fd);
        break ;

    case ReqInfo::NORMAL_BODY:
        this->receiveRequestNormalBody(fd);
        break ;

    case ReqInfo::CHUNKED_BODY:
        this->receiveRequestChunkedBody(fd);
        break ;

    case ReqInfo::MUST_CLEAR:
        this->clearRequestBuffer(fd);
        break ;

    default:
        break ;
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

bool
Server::isFileUri(Request& request)
{
    return (request.getUri().back() != '/');
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
            std::string response_message;
            response_message = this->makeResponseMessage(this->_requests[fd]);
            // TODO: sendResponse error handling
            if (!(sendResponse(response_message, fd)))
                std::cerr<<"Error: sendResponse"<<std::endl;
            this->_server_manager->fdClr(fd, FdSet::WRITE);
            this->_requests[fd].clear();
            this->_responses[fd].init();
        }
        else if (this->_server_manager->fdIsSet(fd, FdSet::READ))
        {
            try
            {
                if (this->isCGIPipe(fd))
                {
                }
                else if (this->isStaticResource(fd))
                {
                }
                else if (this->isClientSocket(fd))
                {
                    this->receiveRequest(fd);
                    if (this->_requests[fd].getReqInfo() == ReqInfo::COMPLETE)
                    {
                        this->findResourceAbsPath(fd);
                        ResType res = this->checkResourceType(fd);
                        std::cout<<"ResType: "<<(int)res<<std::endl;
                        // preprocessing with swith of res;
                    }
                    Log::getRequest(*this, fd);
                }
            }
            catch(const CannotOpenDirectoryException& e)
            {
                std::cerr << e.s_what() << '\n';
                this->_server_manager->fdSet(fd, FdSet::WRITE);
            }
            catch(const Request::RequestFormatException& e)
            {
                if (this->_requests[fd].isContentLeftInBuffer())
                    this->_requests[fd].setReqInfo(ReqInfo::MUST_CLEAR);
                else
                {
                    this->_requests[fd].setReqInfo(ReqInfo::COMPLETE);
                    this->_server_manager->fdSet(fd, FdSet::WRITE);
                }
            }
            catch(const ReadErrorException& e)
            {
                this->closeClientSocket(fd);
                std::cerr << e.what() << '\n';
            }
            catch(const std::exception& e)
            {
                this->closeClientSocket(fd);
                std::cerr << e.what() << '\n';
            }
            catch(const char* e)
            {
                this->closeClientSocket(fd);
                std::cerr << e << '\n';
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

void
Server::findResourceAbsPath(int fd)
{
    UriParser parser;
    parser.parseUri(this->_requests[fd].getUri()); // scheme, host, port, path
    const std::string& path = parser.getPath(); // path

    Response& response = this->_responses[fd];
    response.setRouteAndLocationInfo(path, this); // Response객체에 route주소와 location_info가 저장됨
    std::string root = response.getLocationInfo().at("root");
    if (response.getRoute() != "/")
        root.pop_back();
    std::string file_path = path.substr(response.getRoute().length());
    response.setResourceAbsPath(root + file_path);
    std::cout<<response.getResourceAbsPath()<<std::endl;
}

ResType
Server::checkResourceType(int fd)
{
    // 1. file or folder
    //   - 
    // 맨 마지막 값이 '/' 면 폴더, 아니면 파일
    // 2. 만약 파일이면
    //   일반파일, cgi
    // 3. 폴더
    //   location_info를 확인하여 index 옵션 확인해서 해당하는 파일이 있는지 검사.
    //      만약 있다면 index로 처리
    //           없다면 autoindex 확인. 
    //                  off면 "403" error_code
    //                  on이면 autoindex 처리
    const location_info& location_info = this->_responses[fd].getLocationInfo();
    
    DIR* dir_ptr;

    if ((dir_ptr = opendir(this->_responses[fd].getResourceAbsPath().c_str())) == NULL)
    {
        if (errno == ENOTDIR) // 폴더가 아님
        {
            location_info::const_iterator it = location_info.find("cgi");
            if (it == location_info.end())
                return (ResType::STATIC_RESOURCE);
            size_t dot = this->_responses[fd].getResourceAbsPath().rfind(".");
            if (dot == std::string::npos)
                return (ResType::STATIC_RESOURCE);
            std::string extension = this->_responses[fd].getResourceAbsPath().substr(dot);
            const std::string& cgi = it->second;
            if (cgi.find(extension) == std::string::npos)
                return (ResType::STATIC_RESOURCE);
            return (ResType::CGI);
        }
        else if (errno == EACCES)
            throw CannotOpenDirectoryException(this->_requests[fd], "403");
        else if (errno == ENOENT)
            throw CannotOpenDirectoryException(this->_requests[fd], "404");
    }
    else // 폴더인 경우 일단 폴더에 뭐가 있는지 반드시 찾아봐야 한다
    {
        this->_responses[fd].setDirectoryEntry(dir_ptr);
        if (this->isIndexFileExist())
            return (ResType::INDEX_HTML);
        else
        {
            // AUtoindex 옵션을 검사한다.
            if (this->isAutoIndexOn())
                return (ResType::AUTO_INDEX);
            else
            {
                this->_responses[fd].setStatusCode("403");
                return (ResType::ERROR_CODE);
            }
        }
        
    }
    return (ResType::ERROR_CODE);
}