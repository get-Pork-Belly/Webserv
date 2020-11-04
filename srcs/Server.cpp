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

    response.applyAndCheckRequest(request, this);
    response.makeBody(request);
    // std::cout << "---------- body --------------" << std::endl;
    // std::cout << response.getBody() << std::endl;
    // headers = response.makeHeaders(request);
    status_line = response.makeStatusLine();
    Log::trace("< makeResponseMessage");
    return (status_line + headers + response.getBody());
}

bool
Server::sendResponse(const std::string& response_message, int fd)
{
    Log::trace("> sendResponse");
    std::string tmp;
    // std::string tmp = "fd: ";
    // tmp += std::to_string(fd);
    // tmp += " in send response\n";
    // tmp += "===============================\n";
    // tmp += "response_message\n ";
    // tmp += "===============================\n";
    tmp += response_message;
    tmp += "\r\n";
    std::cout<<tmp<<std::endl;
    // std::cout<<"rm length: "<<response_message.length()<<std::endl;
    int res = write(fd, tmp.c_str(), tmp.length()); 
    std::cout<<"res: "<<res<<std::endl;
    // response_message += "wow";
    // std::cout<<"response_message: "<<response_message<<std::endl;

    // const char* tmp2 = ft::strdup(response_message);
    // std::cout<<"response_message: "<<tmp2<<std::endl;
    // std::cout<<"tmp2 len: "<<ft::strlen(tmp2)<<std::endl;
    // write(fd, response_message.c_str(), response_message.length());
    // write(fd, tmp2, ft::strlen(tmp2));
    // (void)response_message;
    // std::cout<<"fd: "<<fd<<std::endl;
    // errno = 0;
    // int ret;
    // ret = write(fd, "why?\r\n", 4); 
    // ssize_t ret = send(fd, tmp.c_str(), tmp.length(), 0);
    // std::cout<<"ret: "<<ret<<std::endl;
    // int num = errno;
    // std::cout<<"ret: "<<ret<<std::endl;
    // std::cout<<num<< ": "<<strerror(num)<<std::endl;
    Log::trace("< sendResponse");
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
    Log::trace("> isClientSocket");
    const std::vector<std::pair<FdType, int> >& fd_table = this->_server_manager->getFdTable();
    if (fd_table[fd].first == FdType::CLIENT_SOCKET)
        return true;
    Log::trace("< isClientSocket");
    return false;
}

bool
Server::isStaticResource(int fd) const
{
    Log::trace("> isStaticResource");
    const std::vector<std::pair<FdType, int> >& fd_table = this->_server_manager->getFdTable();
    if (fd_table[fd].first == FdType::RESOURCE)
        return true;
    return false;
    Log::trace("< isStaticResource");
}

bool
Server::isCGIPipe(int fd) const
{
    Log::trace("> isCGIPipe");
    const std::vector<std::pair<FdType, int> >& fd_table = this->_server_manager->getFdTable();
    if (fd_table[fd].first == FdType::PIPE)
    {
        return true;
        Log::trace("< isCGIPipe return true");
    }
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
Server::isCgiUri(int fd)
{
    const location_info& location_info = this->_responses[fd].getLocationInfo();

    location_info::const_iterator it = location_info.find("cgi");
    if (it == location_info.end())
        return (false);
    size_t dot = this->_responses[fd].getResourceAbsPath().rfind(".");
    if (dot == std::string::npos)
        return (false);
    std::string extension = this->_responses[fd].getResourceAbsPath().substr(dot);
    const std::string& cgi = it->second;
    if (cgi.find(extension) == std::string::npos)
        return (false);
    return (true);
}

void
Server::acceptClient()
{
    int client_socket;
    struct sockaddr client_address;
    socklen_t client_len = sizeof(client_address);

    if ((client_socket = accept(this->getServerSocket(), &client_address, &client_len)))
    {
        this->_server_manager->fdSet(client_socket, FdSet::READ);

        //TODO: Client IP address 저장하기
        // this->_requests[client_socket].setIpAddress(ft::inetNtoA(client_address.sin_addr.s_addr))

        fcntl(client_socket, F_SETFL, O_NONBLOCK);
        this->_server_manager->setClientSocketOnFdTable(client_socket, this->getServerSocket());
        this->_server_manager->updateFdMax(client_socket);
        Log::newClient(*this, client_socket);
    }
    else
        std::cerr<<"Accept error"<<std::endl;
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
            std::cout << "fd: " << fd << std::endl;
            std::string response_message = this->makeResponseMessage(fd);
            // std::cout << "message: " << response_message << std::endl;
            // response_message = this->makeResponseMessage(this->_requests[fd], fd);
            // TODO: sendResponse error handling
            // std::cout<<response_message.c_str()<<std::endl;
            // const char* tmp = response_message.c_str();
            // write(fd, tmp, strlen(tmp));
            if (!(sendResponse(response_message, fd)))
                std::cerr<<"Error: sendResponse"<<std::endl;
            this->_server_manager->fdClr(fd, FdSet::WRITE);
            this->_requests[fd].clear();
            this->_responses[fd].init();
            Log::trace("<<< write sequence");
        }
        else if (this->_server_manager->fdIsSet(fd, FdSet::READ))
        {
            try
            {
                if (this->isCGIPipe(fd))
                {
                    this->executeCgiAndReadCgiPipe(fd);
                    this->_server_manager->fdClr(fd, FdSet::READ);
                }
                else if (this->isStaticResource(fd))
                {
                    this->readStaticResource(fd);
                }
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

//TODO: 함수명이 기능을 담지 못함, 수정 필요함!
void
Server::findResourceAbsPath(int fd)
{
    Log::trace("> findResourceAbsPath");
    UriParser parser;
    parser.parseUri(this->_requests[fd].getUri());
    const std::string& path = parser.getPath();
    std::cout << "=========================== " << std::endl;
    std::cout << "path: " << path << std::endl;
    std::cout<< "fd: " << fd << std::endl;
    std::cout << "=========================== " << std::endl;

    Response& response = this->_responses[fd];
    response.setRouteAndLocationInfo(path, this);

    std::cout << "=========================== " << std::endl;
    std::cout << "route: " << path << std::endl;
    std::cout<< "fd: " << fd << std::endl;
    std::cout << "=========================== " << std::endl;
    std::string root = response.getLocationInfo().at("root");
    if (response.getRoute() != "/")
        root.pop_back();
    std::string file_path = path.substr(response.getRoute().length());
    response.setResourceAbsPath(root + this->_responses[fd].getRoute() + file_path);
    std::cout<<"in findresourceAbsPath: "<<response.getResourceAbsPath()<<std::endl;
    Log::trace("< findResourceAbsPath");
}

void 
Server::readStaticResource(int fd)
{
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
    // Log::trace("< readStaticResource");
}

void
Server::openStaticResource(int fd)
{
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
}

void
Server::checkAndSetResourceType(int fd)
{
    Log::trace("> checkAndSetResourceType");

    Response& response = this->_responses[fd];
    if (this->isCgiUri(fd))
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
        this->openCgiPipe(fd);
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

    this->findResourceAbsPath(fd);
    this->checkAndSetResourceType(fd);
    if (this->_responses[fd].getResourceType() == ResType::INDEX_HTML)
        this->setResourceAbsPathAsIndex(fd);
    ResType res_type = this->_responses[fd].getResourceType();
    preprocessResponseBody(fd, res_type);

    Log::trace("< processResopnseBody");
}

void
Server::openCgiPipe(int fd) // clienet
{
    this->_responses[fd].openCgiPipe();
    int cgi_pipe_fd = this->_responses[fd].getCgiPipeFd();
    fcntl(cgi_pipe_fd, F_SETFL, O_NONBLOCK);
    this->_server_manager->fdSet(cgi_pipe_fd, FdSet::READ);
    this->_server_manager->setCGIPipeOnFdTable(cgi_pipe_fd, fd);
    this->_server_manager->updateFdMax(cgi_pipe_fd);
}

char**
Server::makeCgiEnvp(int fd)
{
    Log::trace("> makeCgiEnvp");
    char** envp;
    const std::vector<std::pair<FdType, int> >& fd_table = this->_server_manager->getFdTable();
    int client_fd = fd_table.at(fd).second;
    const std::map<std::string, std::string>& headers = this->_requests[client_fd].getHeaders();

    const std::map<std::string, std::string>& location_info =
        this->getLocationConfig().at(this->_responses[client_fd].getRoute());

    // 각각에 대한 it
    std::map<std::string, std::string>::const_iterator it;

    if (!(envp = (char **)malloc(sizeof(char *) * 20)))
        return (nullptr);
    for (int i = 0; i < 20; i++)
        envp[i] = nullptr;

    // std::map<std::string, std::string>::const_iterator it = headers.find("Authorization");
    // AUTH_TYPE // Request_Headers의 Authorization value 공백 앞부분
    // REMOTE_IDENT & REMOTE_USER // Request_Headers의 Authorization value 공백 뒷부분
    // AUTH_TYPE
    // it = headers.find("Authorization");
    // if (it == headers.end())
    //     // envp[0] = ft::strdup("");
    // else
    // {
    //     // envp[0] = ft::strdup("")
    //     // REMOTE_USER; 1
    //     // REMOTE_IDENT 2
    // }
    

    // CONTENT_LENGTH // 리퀘스트 바디가 있을 경우 무조건 일치하는 길이가 세팅되어야 함. 없을 때는 null 또는 unset할 것.
    //NOTE: if no data is attached, then NULL (or unset)
    it = headers.find("Content-Length");
    if (it == headers.end())
    {
        if (!(envp[3] = ft::strdup("")))
            return (nullptr);
    }
    else
    {
        if (!(envp[3] = ft::strdup("CONTENT_LENGTH=" + it->second)))
            return (nullptr);
    }

    // CONTENT_TYPE default는 text/html 나머지는 MIME 타입
    //NOTE: http default is text
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

    // GATEWAY_INTERFACE "CGI/1.1"
    if (!(envp[5] = ft::strdup("GATEWAY_INTERFACE=CGI/1.1")))
        return (nullptr);

    // PATH_INFO / 로 시작하는 cgi 스크립트의 path
    it = location_info.find("cgi_path");
    if (it == location_info.end())
        return (nullptr);
    else
    {
        if (!(envp[6] = ft::strdup("PATH_INFO=" + it->second)))
            return (nullptr);
    }

    // PATH_TRANSLATED는 (query가 없을땐...) PATH_INFO랑 동일한 값으로 세팅
    // if (!(envp[7] = ft::strdup(envp[6])))
    //     return (nullptr);

    // QUERY_STRING의 경우 query가 없으면 ""로 세팅
    // if (!(envp[8] = ft::strdup("")))
        // return (nullptr);

    // REMOTE_ADDR -> 클라이언트 ip
    // TODO: requests에 추가하기

    // REQUEST_METHOD : Location info의 limit_except or GET/POST/HEAD
    if (!(this->_requests[client_fd].getMethod() == "GET" ||
            this->_requests[client_fd].getMethod() == "POST" ||
            this->_requests[client_fd].getMethod() == "HEAD"))
            return (nullptr);
    else
    {
        if (!(envp[10] = ft::strdup("REQUEST_METHOD="+ this->_requests[client_fd].getMethod())))
            return (nullptr);
    }

    // REQUEST_URI -> URI abs PATH no RFC
    if (!(envp[11] = ft::strdup("REQUEST_URI=" + this->_responses[client_fd].getResourceAbsPath())))
        return (nullptr);

    // SCRIPT_NAME -> URI (not url) // no path_info segment
    if (!(envp[12] = ft::strdup("SCRIPT_NAME=" + this->_requests[client_fd].getUri())))
        return (nullptr);

    // SERVER_NAME host -> server_info
    if (!(envp[13] = ft::strdup("SERVER_NAME=" + this->getHost())))
        return (nullptr);
    
    // SERVER_PORT port -> server_info
    if (!(envp[14] = ft::strdup("SERVER_PORT=" + this->getPort())))
        return (nullptr);

    // SERVER_PROTOCOL "HTTP/1.1"
    if (!(envp[15] = ft::strdup("SERVER_PROTOCOL=HTTP/1.1")))
        return (nullptr);
    // SERVER_SOFTWARE "GET_POLAR_BEAR/2.0"
    if (!(envp[16] = ft::strdup("SERVER_SOFTWARE=GET_POLAR_BEAR/2.0")))
        return (nullptr);
    Log::trace("< makeCgiEnvp");
    return (envp);
}

char**
Server::makeCgiArgv(int fd)
{
    Log::trace("> makeCgiArgv");
    char** argv;
    const std::vector<std::pair<FdType, int> >& fd_table = this->_server_manager->getFdTable();
    int client_fd = fd_table.at(fd).second;
    const std::string& body = this->_requests[client_fd].getBodies();

    if (!(argv = (char **)malloc(sizeof(char *) * 2)))
        return (nullptr);

    const location_info& location_info =
            this->getLocationConfig().at(this->_responses[client_fd].getRoute());

    //TODO: ft::strdup 만들기
    if (!(argv[0] = ft::strdup(location_info.at("cgi_path"))))
        return (nullptr);
    if (!(argv[1] = ft::strdup(body)))
        return (nullptr);

    Log::trace("< makeCgiArgv");
    return (argv);
}

void
Server::executeCgiAndReadCgiPipe(int fd)
{
    // pid_t pid;
    // int status;
    // int ret;

    char** argv = makeCgiArgv(fd); // 1-> CGI PATH 2-> request body
    char** envp = makeCgiEnvp(fd);
    std::cout << "================================" << std::endl;
    std::cout << "================================" << std::endl;
    std::cout << "================================" << std::endl;
    for (int i = 0; i < 2; i ++)
    {
        std::cout << "argv[" << i << "] " << argv[i] << std::endl;
    }
    for (int i = 0; i < 20; i++)
    {
        if (envp[i])
            std::cout << "ENVP[" << i << "] " << envp[i] << std::endl;
    }
    std::cout << "================================" << std::endl;
    std::cout << "================================" << std::endl;
    std::cout << "================================" << std::endl;

    // pid = fork();
    // //TODO: signal
    // if (pid < 0)
    // {
    //     throw();
    // }
    // else if (pid == 0)
    // {
    //     //dup2
    //     if ((dup2(this->_responses[fd].getCgiPipeFd(), 1)) < 0)
    //     {
    //         throw();
    //     }
    //     if ((ret = execve(cgi_path, cgi_path + request_body, envp)) < 0)
    //     {
    //         // execve(arg[0], arg, envp);
    //     }
    //     exit(ret);
    // }
    // else
    // {
    //     waitpid(pid, &status, 0);
    //     //close
    // }
    //1. pid로 fork
    //2. 자식 프로세스에서는 cgi path, request_body, 환경변수를 인자로 넘겨서 exeve해야함.
    //3. 부모 프로세스에서는 cgi 실행값을 pipe에서 read하여 body에 저장할 것.

    // TODO: cgi execute 끝나면 클라이언트 소켓 WRITE flag 세워주기.
    this->closeFdAndSetClientOnWriteFdSet(fd);
}
