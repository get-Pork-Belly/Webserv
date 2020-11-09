#include "Server.hpp"
#include "utils.hpp"
#include "ServerManager.hpp"
#include "Log.hpp"
#include "UriParser.hpp"
#include <iostream>
#include <signal.h>

/*============================================================================*/
/****************************  Static variables  ******************************/
/*============================================================================*/

/*============================================================================*/
/******************************  Constructor  *********************************/
/*============================================================================*/

Server::Server(ServerManager* server_manager, server_info& server_config, std::map<std::string, location_info>& location_config)
: _server_manager(server_manager), _server_config(server_config),
_server_socket(-1), _server_name(""), _host(server_config["server_name"]), _port(""),
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

const std::string&
Server::getHost() const
{
    return (this->_host);
}

const std::string&
Server::getPort() const
{
    return (this->_port);
}

/*============================================================================*/
/********************************  Setter  ************************************/
/*============================================================================*/

void
Server::setResourceAbsPathAsIndex(int fd)
{
    Response& response = this->_responses[fd];
    const std::string& dir_entry = response.getDirectoryEntry();
    const location_info& location_info = response.getLocationInfo();
    std::vector<std::string> indexs = ft::split(location_info.at("index"), " ");

    const std::string& path = response.getResourceAbsPath();
    for (std::string& index : indexs)
    {
        if (dir_entry.find(index) != std::string::npos)
        {
            response.setResourceType(ResType::STATIC_RESOURCE);
            response.setResourceAbsPath(path + index);
        }
    }
}

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

Server::CannotOpenDirectoryException::CannotOpenDirectoryException(Response& res, const std::string& status_code, int error_num)
: _res(res), _error_num(error_num), _msg("CannotOpenDirectoryException: " + std::string(strerror(_error_num)))
{
    this->_res.setStatusCode(status_code);
}

const char*
Server::CannotOpenDirectoryException::what() const throw()
{
    return (this->_msg.c_str());
}

Server::OpenResourceErrorException::OpenResourceErrorException(Response& response, int error_num)
: _response(response), _error_num(error_num), _msg("OpenResourceErrorException: " + std::string(strerror(this->_error_num)))
{
    if (this->_error_num == EACCES)
        this->_response.setStatusCode("403");
    else if (this->_error_num == ENOMEM)
        this->_response.setStatusCode("500");
    else
        this->_response.setStatusCode("404");
}

const char*
Server::OpenResourceErrorException::what() const throw()
{
    return (this->_msg.c_str());
}

Server::IndexNoExistException::IndexNoExistException(Response& response)
: _response(response)
{
    this->_response.setStatusCode("403");
}

const char*
Server::IndexNoExistException::what() const throw()
{
    return ("[CODE 403] No index & Autoindex off");
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
    std::cout << "Server addres: " << this->_server_address.sin_addr.s_addr << std::endl;

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
    Log::trace("> readBufferUntilHeaders");
    int bytes;
    Request& req = this->_requests[fd];

    if ((bytes = read(fd, buf, header_end_pos + 4)) > 0)
        req.parseRequestWithoutBody(buf);
    else if (bytes == 0)
        throw (Request::RequestFormatException(req, "400"));
    else
        throw (ReadErrorException());
    Log::trace("< readBufferUntilHeaders");
}

void
Server::receiveRequestWithoutBody(int fd)
{
    Log::trace("> receiveRequestWithoutBody");
    int bytes;
    char buf[BUFFER_SIZE + 1];
    size_t header_end_pos = 0;
    Request& req = this->_requests[fd];

    ft::memset(reinterpret_cast<void *>(buf), 0, BUFFER_SIZE + 1);
    if ((bytes = recv(fd, buf, BUFFER_SIZE, MSG_PEEK)) > 0)
    {
        if ((header_end_pos = std::string(buf).find("\r\n\r\n")) != std::string::npos)
        {
            if (static_cast<size_t>(bytes) == header_end_pos + 4)
            {
                req.setIsBufferLeft(false);
                req.setReqInfo(ReqInfo::COMPLETE);
            }
            else
                req.setIsBufferLeft(true);
            this->readBufferUntilHeaders(fd, buf, header_end_pos);
        }
        else
        {
            //NOTE: if BUFFER_SIZE is too small to read including "\r\n\r\n", this block always execute. 
            req.setIsBufferLeft(true);
            throw (Request::RequestFormatException(req, "400"));
        }
    }
    else if (bytes == 0)
        this->closeClientSocket(fd);
    else
        throw (ReadErrorException());
    Log::trace("< receiveRequestWithoutBody");
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
        // TODO cgi를 읽기전에 client의 write 플래그가 세워짐. 다음 select에서 client가 먼저 선택되고 리퀘스트가 초기화 되버림.
        // this->_server_manager->fdSet(fd, FdSet::WRITE);
    }
    else if (bytes == 0)
        this->closeClientSocket(fd);
    else
        throw (ReadErrorException());
}

void
Server::clearRequestBuffer(int fd)
{
    Log::trace("> clearRequestBuffer");
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
    Log::trace("< clearRequestBuffer");
}

void
Server::receiveRequestChunkedBody(int fd)
{
    Log::trace("> receiveRequestChunkedBody");
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
    Log::trace("< receiveRequestChunkedBody");
}

void
Server::receiveRequest(int fd)
{
    Log::trace("> receiveRequest");
    const ReqInfo& req_info = this->_requests[fd].getReqInfo();

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
    Log::trace("< receiveRequest");
}

std::string
Server::makeResponseMessage(int fd)
{
    Log::trace("> makeResponseMessage");
    Request& request = this->_requests[fd];
    Response& response = this->_responses[fd];

    std::string status_line;
    std::string headers;

    //TODO: parsing 할 때 method 허용여부 확인하여 throw, response 꾸미기
    // response.applyAndCheckRequest(request, this);
    response.makeBody(request);
    headers = response.makeHeaders(request);
    status_line = response.makeStatusLine();
    Log::trace("< makeResponseMessage");
    return (status_line + headers + response.getBody());
}

bool
Server::sendResponse(const std::string& response_message, int fd)
{
    Log::trace("> sendResponse");
    std::string tmp;
    tmp += response_message;
    tmp += "\r\n";
    std::cout<<tmp<<std::endl;
    write(fd, tmp.c_str(), tmp.length()); 
    Log::trace("< sendResponse");
    return (true);
}

bool
Server::isFdManagedByServer(int fd) const
{
    // Log::trace("> isFdManagedByServer");
    const std::vector<std::pair<FdType, int> >& fd_table = this->_server_manager->getFdTable();
    // std::cout << "FdType: " << Log::fdTypeToString(this->_server_manager->getFdType(fd)) << std::endl;
    // std::cout << "Fd: " << fd << std::endl;

    if (fd_table[fd].first == FdType::CLIENT_SOCKET)
    {
        // Log::trace("< isFdManagedByServer");
        return (isClientOfServer(fd));
    }
    else if (fd_table[fd].first == FdType::RESOURCE || fd_table[fd].first == FdType::PIPE)
    {
        // Log::trace("< isFdManagedByServer");
        return (isClientOfServer(fd_table[fd].second));
    }
    // Log::trace("< isFdManagedByServer");
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
    Log::trace("> isServerSocket");
    const std::vector<std::pair<FdType, int> >& fd_table = this->_server_manager->getFdTable();
    if (fd_table[fd].first == FdType::SERVER_SOCKET)
        return true;
    return false;
}

bool
Server::isClientSocket(int fd) const
{
    Log::trace("> isClientSocket");
    const std::vector<std::pair<FdType, int> >& fd_table = this->_server_manager->getFdTable();
    if (fd_table[fd].first == FdType::CLIENT_SOCKET)
    {
        Log::trace("< isClientSocket return true");
        return true;
    }
    Log::trace("< isClientSocket return false");
    return false;
}

bool
Server::isStaticResource(int fd) const
{
    Log::trace("> isStaticResource");
    const std::vector<std::pair<FdType, int> >& fd_table = this->_server_manager->getFdTable();
    if (fd_table[fd].first == FdType::RESOURCE)
    {
        Log::trace("< isStaticResource return true");
        return true;
    }
    Log::trace("< isStaticResource return false");
    return false;
}

bool
Server::isCGIPipe(int fd) const
{
    Log::trace("> isCGIPipe");
    const std::vector<std::pair<FdType, int> >& fd_table = this->_server_manager->getFdTable();
    if (fd_table[fd].first == FdType::PIPE)
    {
        Log::trace("< isCGIPipe return true");
        return true;
    }
    Log::trace("< isCGIPipe return false");
    return false;
}


bool
Server::isFileUri(const Request& request) const
{
    return (request.getUri().back() != '/');
}

bool
Server::isIndexFileExist(int fd)
{
    const std::string& dir_entry = this->_responses[fd].getDirectoryEntry();
    const location_info& location_info = this->_responses[fd].getLocationInfo();
    std::vector<std::string> indexs = ft::split(location_info.at("index"), " ");
    for (std::string& index : indexs)
    {
        if (dir_entry.find(index) != std::string::npos)
            return (true);
    }
    return (false);
}

bool
Server::isAutoIndexOn(int fd)
{
    const location_info& location_info = this->_responses[fd].getLocationInfo();
    Log::printLocationInfo(location_info);
    if (location_info.at("autoindex") == "on")
        return (true);
    return (false);
}

bool
Server::isCgiUri(int fd, const std::string& extension)
{
    Log::trace("> isCgiUri");

    if (extension == "")
    {
        Log::trace("< isCgiUri return false");
        return (false);
    }

    const location_info& location_info = this->_responses[fd].getLocationInfo();
    location_info::const_iterator it = location_info.find("cgi");
    if (it == location_info.end())
    {
        Log::trace("< isCgiUri return false");
        return (false);
    }

    const std::string& cgi = it->second;
    if (cgi.find(extension) == std::string::npos)
    {
        Log::trace("< isCgiUri return false");
        return (false);
    }

    Log::trace("< isCgiUri return true");
    return (true);
}

void
Server::acceptClient()
{
    Log::trace("> acceptClient");
    int client_socket;
    struct sockaddr_in client_address;
    socklen_t client_len = sizeof(client_address);

    if ((client_socket = accept(this->getServerSocket(), reinterpret_cast<sockaddr*>(&client_address), &client_len)))
    {
        this->_server_manager->fdSet(client_socket, FdSet::READ);
        this->_requests[client_socket].setIpAddress(ft::inetNtoA(client_address.sin_addr.s_addr));
        fcntl(client_socket, F_SETFL, O_NONBLOCK);
        this->_server_manager->setClientSocketOnFdTable(client_socket, this->getServerSocket());
        this->_server_manager->updateFdMax(client_socket);
        Log::newClient(*this, client_socket);
    }
    else
        std::cerr<<"Accept error"<<std::endl;
    Log::trace("< acceptClient");
}

void
Server::sendDataToCgi(int write_fd_to_cgi)
{
    //NOTE: FD는 writeFdToCGI
    Log::trace("> sendDataToCgi");
    int bytes;
    int client_fd;
    int content_length;
    int transfered;
    char* body;

    bytes = 0;
    client_fd = this->_server_manager->getConnectedFd(write_fd_to_cgi);
    Request& request = this->_requests[client_fd];
    Response& response = this->_responses[client_fd];
    content_length = request.getContentLength();
    transfered = request.getTransfered();
    //NOTE 이 부분에서 문제가 속도 이슈가 생길수도 있음.
    body = ft::strdup(request.getBodies());
    bytes = write(write_fd_to_cgi, &body[transfered], content_length);
    free(body);

    if (bytes > 0)
    {
        transfered += bytes;
        request.setTransfered(transfered);
        if (transfered == content_length)
            this->closeFdAndSetFd(write_fd_to_cgi, FdSet::WRITE, response.getReadFdFromCGI(), FdSet::READ);
    }
    else if (bytes == 0)
        throw "write error"; //error 500
    else
        throw "write error"; //error 500

    Log::trace("< sendDataToCgi");
}

void
Server::receiveDataFromCgi(int read_fd_from_cgi)
{
    Log::trace("> receiveDataFromCgi");
    int bytes;
    int client_fd;
    int status;
    char buf[BUFFER_SIZE + 1];

    client_fd = this->_server_manager->getConnectedFd(read_fd_from_cgi);
    Response& response = this->_responses[client_fd];

    ft::memset(static_cast<void *>(buf), 0, BUFFER_SIZE + 1);
    bytes = read(read_fd_from_cgi, buf, BUFFER_SIZE + 1);
    if (bytes > 0)
    {
        response.setBody(buf);
        this->closeFdAndSetFd(read_fd_from_cgi, FdSet::READ, client_fd, FdSet::WRITE);
        //NOTE waitpid의 타이밍을 잘 잡자.
        waitpid(response.getCgiPid(), &status, 0);
    }
    else if (bytes == 0)
        std::cout << "read end!" << std::endl;
    else
        throw("cgi read error");
    Log::trace("< receiveDataFromCgi");
}

void
Server::run(int fd)
{
    if (isServerSocket(fd))
        this->acceptClient();
    else
    {
        if (this->_server_manager->fdIsSet(fd, FdSet::WRITE))
        {
            Log::trace(">>> write sequence");
            if (this->isCGIPipe(fd))
                sendDataToCgi(fd);
            else
            {
                std::string response_message = this->makeResponseMessage(fd);
                // std::cout << "message: " << response_message << std::endl;
                // response_message = this->makeResponseMessage(this->_requests[fd], fd);
                // TODO: sendResponse error handling
                if (!(sendResponse(response_message, fd)))
                    std::cerr<<"Error: sendResponse"<<std::endl;
                this->_server_manager->fdClr(fd, FdSet::WRITE);
                this->_requests[fd].clear();
                this->_responses[fd].init();
            }
            Log::trace("<<< write sequence");
        }
        else if (this->_server_manager->fdIsSet(fd, FdSet::READ))
        {
            try
            {
                std::cout << "Fd: " << fd << "FdType: " << Log::fdTypeToString(this->_server_manager->getFdType(fd)) << std::endl;
                if (this->isCGIPipe(fd)) 
                    receiveDataFromCgi(fd);
                else if (this->isStaticResource(fd))
                    this->readStaticResource(fd);
                else if (this->isClientSocket(fd))
                {
                    this->receiveRequest(fd);
                    if (this->_requests[fd].getReqInfo() == ReqInfo::COMPLETE)
                        processResponseBody(fd);
                    Log::getRequest(*this, fd);
                }
            }
            catch(const SendErrorCodeToClientException& e)
            {
                std::cerr << e.what() << '\n';
                this->_server_manager->fdSet(fd, FdSet::WRITE);
            }
            catch(const Request::RequestFormatException& e)
            {
                std::cerr << e.what() << '\n';
                if (this->_requests[fd].isContentLeftInBuffer())
                    this->_requests[fd].setReqInfo(ReqInfo::MUST_CLEAR);
                else
                {
                    this->_requests[fd].setReqInfo(ReqInfo::COMPLETE);
                    this->_server_manager->fdSet(fd, FdSet::WRITE);
                }
                this->_responses[fd].setStatusCode(this->_requests[fd].getStatusCode());
            }
            catch(const std::exception& e)
            {
                this->closeClientSocket(fd);
                std::cerr << e.what() << '\n';
            }
        }
    }
}

void
Server::closeClientSocket(int fd)
{
    this->_server_manager->fdClr(fd, FdSet::READ);
    this->_server_manager->setClosedFdOnFdTable(fd);
    this->_server_manager->updateFdMax(fd);
    this->_requests[fd].clear();
    Log::closeClient(*this, fd);
    close(fd);
}

void
Server::closeFdAndSetClientOnWriteFdSet(int fd)
{
    const FdType& type = this->_server_manager->getFdTable()[fd].first;
    int client_socket = this->_server_manager->getFdTable()[fd].second;
    Log::closeFd(*this, client_socket, type, fd);

    this->_server_manager->fdClr(fd, FdSet::READ);
    this->_server_manager->setClosedFdOnFdTable(fd);
    this->_server_manager->updateFdMax(fd);
    this->_server_manager->fdSet(client_socket, FdSet::WRITE);
    close(fd);
}

void
Server::closeFdAndSetFd(int clear_fd, FdSet clear_fd_set, int set_fd, FdSet set_fd_set)
{
    Log::trace("> closeFdAndSetFd");
    const FdType& type = this->_server_manager->getFdTable()[clear_fd].first;
    Log::closeFd(*this, set_fd, type, clear_fd);

    this->_server_manager->fdClr(clear_fd, clear_fd_set);
    this->_server_manager->setClosedFdOnFdTable(clear_fd);
    this->_server_manager->updateFdMax(clear_fd);
    this->_server_manager->fdSet(set_fd, set_fd_set);
    close(clear_fd);
    Log::trace("< closeFdAndSetFd");
}

//TODO: 함수명이 기능을 담지 못함, 수정 필요함!
void
Server::findResourceAbsPath(int fd)
{
    Log::trace("> findResourceAbsPath");
    UriParser parser;
    parser.parseUri(this->_requests[fd].getUri());
    const std::string& path = parser.getPath();

    Response& response = this->_responses[fd];
    response.setPath(path);
    response.setRouteAndLocationInfo(path, this);

    std::string root = response.getLocationInfo().at("root");
    if (response.getRoute() != "/")
        root.pop_back();
    std::string file_path = path.substr(response.getRoute().length());
    response.setResourceAbsPath(root + file_path);
    Log::trace("< findResourceAbsPath");
}

void 
Server::readStaticResource(int fd)
{
    Log::trace("> readStaticResouce");
    // Log::trace("> readStaticResource");
    char buf[BUFFER_SIZE + 1];
    int bytes;
    int client_socket = this->_server_manager->getFdTable()[fd].second;

    ft::memset(buf, 0, BUFFER_SIZE + 1);
    if ((bytes = read(fd, buf, BUFFER_SIZE)) > 0)
    {
        this->_responses[client_socket].appendBody(buf);
        if (bytes < BUFFER_SIZE)
            this->closeFdAndSetClientOnWriteFdSet(fd);
    }
    else if (bytes == 0)
    {
        this->closeFdAndSetClientOnWriteFdSet(fd);
        throw (ReadErrorException());
    }
    else
    {
        this->closeFdAndSetClientOnWriteFdSet(fd);
        throw (ReadErrorException());
    }
    Log::trace("< readStaticResource");
}

void
Server::openStaticResource(int fd)
{
    Log::trace("> openStaticResource");
    int resource_fd;
    const std::string& path = this->_responses[fd].getResourceAbsPath();
    struct stat tmp;

    if ((resource_fd = open(path.c_str(), O_RDWR, 0644)) > 0)
    {
        fcntl(resource_fd, F_SETFL, O_NONBLOCK);
        this->_server_manager->fdSet(resource_fd, FdSet::READ);
        this->_server_manager->setResourceOnFdTable(resource_fd, fd);
        this->_server_manager->updateFdMax(resource_fd);
        if ((fstat(resource_fd, &tmp)) == -1)
            throw OpenResourceErrorException(this->_responses[fd], errno);
        this->_responses[fd].setFileInfo(tmp);
    }
    else
        throw OpenResourceErrorException(this->_responses[fd], errno);
    Log::trace("< openStaticResource");
}

void
Server::checkAndSetResourceType(int fd)
{
    Log::trace("> checkAndSetResourceType");

    Response& response = this->_responses[fd];
    response.findAndSetUriExtension();
    if (this->isCgiUri(fd, response.getUriExtension()))
    {
        response.setResourceType(ResType::CGI);
        return ;
    }
    DIR* dir_ptr;
    if ((dir_ptr = opendir(response.getResourceAbsPath().c_str())) == NULL)
    {
        if (errno == ENOTDIR)
            response.setResourceType(ResType::STATIC_RESOURCE);
        else if (errno == EACCES)
            throw (CannotOpenDirectoryException(this->_responses[fd], "403", errno));
        else if (errno == ENOENT)
            throw (CannotOpenDirectoryException(this->_responses[fd], "404", errno));
    }
    else
    {
        response.setDirectoryEntry(dir_ptr);
        closedir(dir_ptr);
        if (this->isIndexFileExist(fd))
            response.setResourceType(ResType::INDEX_HTML);
        else
        {
            std::cout<<"in checkAndSetResourceType fd:"<<fd<<std::endl;
            if (this->isAutoIndexOn(fd))
            {
                response.setResourceType(ResType::AUTO_INDEX);
                this->_server_manager->fdSet(fd, FdSet::WRITE);
            }
            else
                throw (IndexNoExistException(this->_responses[fd]));
        }
    }

    Log::trace("< checkAndSetResourceType");
}

void
Server::preprocessResponseBody(int fd, ResType& res_type)
{
    Log::trace("> preprocessResponseBody");
    switch (res_type)
    {
    case ResType::AUTO_INDEX:
        std::cout << "Auto index page will be generated" << std::endl;
        break ;
    case ResType::STATIC_RESOURCE:
        std::cout << "Static resource will be opened" << std::endl;
        this->openStaticResource(fd);
        break ;
    case ResType::CGI:
        std::cout << "CGIpipe will be opened" << std::endl;
        this->openCgiPipe(fd); //fd: client fd, cgiPipe[2]
        this->forkAndExecuteCgi(fd);
        // write flag -> CGI 프로세스에 스탠다드 인(pipe[1])으로 데이터를 넣어줌.
        // Client fd에 대해서 clear 해준다.
        break ;
    default:
        break ;
    }
    Log::trace("< preprocessResponseBody");
}

void
Server::processResponseBody(int fd)
{
    Log::trace("> processResopnseBody");

    std::cout<<"uri: "<<this->_requests[fd].getUri()<<std::endl;
    this->findResourceAbsPath(fd);
    this->checkAndSetResourceType(fd);
    if (this->_responses[fd].getResourceType() == ResType::INDEX_HTML)
        this->setResourceAbsPathAsIndex(fd);
    ResType res_type = this->_responses[fd].getResourceType();
    preprocessResponseBody(fd, res_type);

    Log::trace("< processResopnseBody");
}

void
Server::openCgiPipe(int client_fd)
{
    Log::trace("> openCgiPipe");
    Response& response = this->_responses[client_fd];
    int pipe1[2];
    int pipe2[2];

    //TODO: 예외객체
    if (pipe(pipe1) < 0)
        return ;
    if (pipe(pipe2) < 0)
        return ;

    int stdin_of_cgi = pipe1[0];
    int stdout_of_cgi = pipe2[1];
    int read_fd_from_cgi = pipe2[0];
    int write_fd_to_cgi = pipe1[1];

    response.setStdinOfCGI(stdin_of_cgi);
    response.setStdoutOfCGI(stdout_of_cgi);
    response.setReadFdFromCGI(read_fd_from_cgi);
    response.setWriteFdToCGI(write_fd_to_cgi);

    fcntl(stdin_of_cgi, F_SETFL, O_NONBLOCK);
    fcntl(stdout_of_cgi, F_SETFL, O_NONBLOCK);
    fcntl(read_fd_from_cgi, F_SETFL, O_NONBLOCK);
    fcntl(write_fd_to_cgi, F_SETFL, O_NONBLOCK);

    this->_server_manager->setCGIPipeOnFdTable(read_fd_from_cgi, client_fd);
    this->_server_manager->setCGIPipeOnFdTable(write_fd_to_cgi, client_fd);
    this->_server_manager->updateFdMax(read_fd_from_cgi);
    this->_server_manager->updateFdMax(write_fd_to_cgi);
    Log::trace("< openCgiPipe");
}

char**
Server::makeCgiEnvp(int client_fd)
{
    Log::trace("> makeCgiEnvp");
    char** envp;
    const std::map<std::string, std::string>& headers = this->_requests[client_fd].getHeaders();
    const std::map<std::string, std::string>& location_info =
        this->getLocationConfig().at(this->_responses[client_fd].getRoute());

    // 각각에 대한 it
    if (!(envp = (char **)malloc(sizeof(char *) * 18)))
        return (nullptr);
    for (int i = 0; i < 18; i++)
        envp[i] = nullptr;

    std::map<std::string, std::string>::const_iterator it = headers.find("Authorization");
    // AUTH_TYPE // Request_Headers의 Authorization value 공백 앞부분
    // REMOTE_IDENT & REMOTE_USER // Request_Headers의 Authorization value 공백 뒷부분
    // AUTH_TYPE
    it = headers.find("Authorization");
    if (it == headers.end())
    {
        if (!(envp[0] = ft::strdup("AUTH_TYPE=")))
            return (nullptr);
    }
    else
    {
        if (!(envp[0] = ft::strdup("AUTH_TYPE=")))
            return (nullptr);
    }
        // REMOTE_USER; 1
    if (!(envp[1] = ft::strdup("REMOTE_USER=")))
        return (nullptr);
        // REMOTE_IDENT 2
    if (!(envp[2] = ft::strdup("REMOTE_IDENT=")))
        return (nullptr);

    it = headers.find("Content-Length");
    if (it == headers.end())
    {
        if (!(envp[3] = ft::strdup("CONTENT_LENGTH=")))
            return (nullptr);
    }
    else
    {
        if (!(envp[3] = ft::strdup("CONTENT_LENGTH=" + it->second)))
            return (nullptr);
    }

    it = headers.find("Content-Type");
    if (it == headers.end())
    {
        if (!(envp[4] = ft::strdup("CONTENT_TYPE=text/html")))
            return (nullptr);
    }
    else
    {
        if (!(envp[4] = ft::strdup("CONTENT_TYPE=" + it->second)))
            return (nullptr);
    }

    if (!(envp[5] = ft::strdup("GATEWAY_INTERFACE=CGI/1.1")))
        return (nullptr);
    if (!(envp[6] = ft::strdup("PATH_INFO=" + this->_responses[client_fd].getPath())))
        return (nullptr);
    if (!(envp[7] = ft::strdup("PATH_TRANSLATED=" + this->_responses[client_fd].getResourceAbsPath())))
        return (nullptr);
    //TODO: GET일 때는 QUERY를 여기로 넣어주기
    if (!(envp[8] = ft::strdup("QUERY_STRING=")))
        return (nullptr);
    if (!(envp[9] = ft::strdup("REMOTE_ADDR=" + this->_requests[client_fd].getIpAddress())))
        return (nullptr);
    //TODO: get/head <-> post 구조 다르게 가져가야함.  REQUEST_METHOD : Location info의 limit_except or GET/POST/HEAD
    if (!(this->_requests[client_fd].getMethod() == "GET" ||
            this->_requests[client_fd].getMethod() == "POST" ||
            this->_requests[client_fd].getMethod() == "HEAD"))
        return (nullptr);
    else
    {
        if (!(envp[10] = ft::strdup("REQUEST_METHOD="+ this->_requests[client_fd].getMethod())))
            return (nullptr);
    }
    if (!(envp[11] = ft::strdup("REQUEST_URI=" + this->_responses[client_fd].getPath())))
        return (nullptr);
    if (!(envp[12] = ft::strdup("SCRIPT_NAME=" + location_info.at("cgi_path"))))
        return (nullptr);
    if (!(envp[13] = ft::strdup("SERVER_NAME=" + this->getHost())))
        return (nullptr);
    if (!(envp[14] = ft::strdup("SERVER_PORT=" + this->getPort())))
        return (nullptr);
    if (!(envp[15] = ft::strdup("SERVER_PROTOCOL=HTTP/1.1")))
        return (nullptr);
    if (!(envp[16] = ft::strdup("SERVER_SOFTWARE=GET_POLAR_BEAR/2.0")))
        return (nullptr);
    Log::trace("< makeCgiEnvp");
    return (envp);
}

char**
Server::makeCgiArgv(int client_fd)
{
    Log::trace("> makeCgiArgv");
    char** argv;
    Response& response = this->_responses[client_fd];

    if (!(argv = (char **)malloc(sizeof(char *) * 3)))
        return (nullptr);
    const location_info& location_info =
            this->getLocationConfig().at(this->_responses[client_fd].getRoute());
    if (!(argv[0] = ft::strdup(location_info.at("cgi_path"))))
        return (nullptr);
    if (!(argv[1] = ft::strdup(response.getResourceAbsPath())))
        return (nullptr);
    argv[2] = nullptr;
    Log::trace("< makeCgiArgv");
    return (argv);
}

void
Server::forkAndExecuteCgi(int client_fd)
{
    Log::trace("> forkAndExecuteCgi");

    Response& response = this->_responses[client_fd];
    // Request& request = this->_requests[client_fd];
    int stdin_of_cgi = response.getStdinOfCGI();
    int stdout_of_cgi = response.getStdoutOfCGI();
    char **argv = this->makeCgiArgv(client_fd);
    char **envp = this->makeCgiEnvp(client_fd);
    pid_t pid;
    int ret;

    //TODO: Cgi Exception 만들기
    if ((pid = fork()) < 0)
        throw ("fork failed");
    else if (pid == 0)
    {
        if (dup2(stdin_of_cgi, 0) < 0)
            throw ("dup2 STDIN error");
        if (dup2(stdout_of_cgi, 1) < 0)
            throw ("dup2 STDOUT error");
        if ((ret = execve(argv[0], argv, envp)) < 0)
            exit(ret);
        exit(ret);
    }
    else
    {
        response.setCgiPid(pid);
        ft::doubleFree(argv);
        ft::doubleFree(envp);
        // NOTE 정상적으로 읽으면 select 알아서 clear 해준다.
        this->_server_manager->fdSet(response.getWriteFdToCGI(), FdSet::WRITE);
    }
    Log::trace("< forkAndExecuteCgi");
}