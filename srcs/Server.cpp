#include "Server.hpp"
#include "utils.hpp"
#include "ServerManager.hpp"
#include "Log.hpp"
#include "UriParser.hpp"
#include "Base64.hpp"
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

Server::MustRedirectException::MustRedirectException(Response& res)
: _res(res), _msg("MustRedirectException: [CODE " + res.getRedirectStatusCode() + "]")
{
    this->_res.setStatusCode(res.getRedirectStatusCode());
}

const char*
Server::MustRedirectException::what() const throw()
{
    return (this->_msg.c_str());
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

Server::CgiExecuteErrorException::CgiExecuteErrorException(Response& response)
: _response(response)
{
    this->_response.setStatusCode("500");
}

const char*
Server::CgiExecuteErrorException::what() const throw()
{
    return ("[CODE 500] Server Internal error");
}

Server::AuthenticateErrorException::AuthenticateErrorException(Response& res, const std::string& status_code)
: _res(res), _status_code(status_code)
{
    this->_res.setStatusCode(this->_status_code);
}

const char*
Server::AuthenticateErrorException::what() const throw()
{
    return ("");
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
    Log::trace("> receiveRequestNormalBody");

    int bytes;
    Request& req = this->_requests[fd];

    int content_length = req.getContentLength();
    if (content_length > this->_limit_client_body_size)
        throw (PayloadTooLargeException(req));

    char buf[BUFFER_SIZE + 1];
    ft::memset(reinterpret_cast<void *>(buf), 0, BUFFER_SIZE + 1);

    if ((bytes = recv(fd, buf, content_length, 0)) > 0)
        req.parseNormalBodies(buf);
    else if (bytes == 0)
        this->closeClientSocket(fd);
    else
        throw (ReadErrorException());

    Log::trace("< receiveRequestNormalBody");
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
        req.parseChunkedBody(buf);
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

    if (response.getStatusCode().compare("200") != 0)
        response.setResourceType(ResType::ERROR_HTML);

    if (response.isRedirection(response.getStatusCode()) == false)
        response.makeBody(request);
    headers = response.makeHeaders(request);
    status_line = response.makeStatusLine();
    Log::trace("< makeResponseMessage");
    // if (request.getMethod().compare("HEAD") == 0)
    //     return (status_line + headers);
    // else if (response.isNeedTobeChunkedBody)
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
Server::isCGIUri(int fd, const std::string& extension)
{
    Log::trace("> isCGIUri");

    if (extension == "")
    {
        Log::trace("< isCGIUri return false");
        return (false);
    }

    const location_info& location_info = this->_responses[fd].getLocationInfo();
    location_info::const_iterator it = location_info.find("cgi");
    if (it == location_info.end())
    {
        Log::trace("< isCGIUri return false");
        return (false);
    }

    const std::string& cgi = it->second;
    if (cgi.find(extension) == std::string::npos)
    {
        Log::trace("< isCGIUri return false");
        return (false);
    }

    Log::trace("< isCGIUri return true");
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
Server::sendDataToCGI(int write_fd_to_cgi)
{
    //NOTE: FD는 writeFdToCGI
    Log::trace("> sendDataToCGI");
    int bytes;
    int client_fd;
    int content_length;
    int transfered_body_size;
    const char* body;

    bytes = 0;
    client_fd = this->_server_manager->getLinkedFdFromFdTable(write_fd_to_cgi);
    Request& request = this->_requests[client_fd];
    Response& response = this->_responses[client_fd];
    content_length = request.getContentLength();
    //TODO: 만약 Content_length가 0 이 아니면서 GET, HEAD로 온 요청이라면
    // 이미 앞에서 걸러질것이므로 if의 조건에 != "POST" 가 있을 필요가 없다.
    // 확실하게 앞에서 걸러 진다면 TODO를 지우자.
    if (content_length == 0 || request.getMethod() != "POST")
    {
        this->closeFdAndSetFd(write_fd_to_cgi, FdSet::WRITE, response.getReadFdFromCGI(), FdSet::READ);
        return ;
    }
    transfered_body_size = request.getTransferedBodySize();
    body = request.getBodies().c_str();
    bytes = write(write_fd_to_cgi, &body[transfered_body_size], content_length);
    if (bytes > 0)
    {
        transfered_body_size += bytes;
        request.setTransferedBodySize(transfered_body_size);
        if (transfered_body_size == content_length)
            this->closeFdAndSetFd(write_fd_to_cgi, FdSet::WRITE, response.getReadFdFromCGI(), FdSet::READ);
    }
    else if (bytes == 0)
        throw (CgiExecuteErrorException(this->_responses[client_fd]));
    else
        throw (CgiExecuteErrorException(this->_responses[client_fd]));

    Log::trace("< sendDataToCGI");
}

void
Server::receiveDataFromCGI(int read_fd_from_cgi)
{
    Log::trace("> receiveDataFromCGI");
    int bytes;
    int client_fd;
    int status;
    char buf[BUFFER_SIZE + 1];

    client_fd = this->_server_manager->getLinkedFdFromFdTable(read_fd_from_cgi);
    Response& response = this->_responses[client_fd];

    ft::memset(static_cast<void *>(buf), 0, BUFFER_SIZE + 1);
    bytes = read(read_fd_from_cgi, buf, BUFFER_SIZE + 1);
    if (bytes > 0)
    {
        response.appendBody(buf);

        //NOTE: BUFFER SIZE보다 읽은 것이 같거나 컸으면, 다시 한 번 버퍼를 확인해 보아야 함.
        if (bytes < BUFFER_SIZE)
        {
            this->closeFdAndSetFd(read_fd_from_cgi, FdSet::READ, client_fd, FdSet::WRITE);
        //NOTE waitpid의 타이밍을 잘 잡자.
            waitpid(response.getCGIPid(), &status, 0);
        }
    }
    //TODO: return 0 확인하기
    else if (bytes == 0)
        throw (CgiExecuteErrorException(this->_responses[client_fd]));
    else
        throw (CgiExecuteErrorException(this->_responses[client_fd]));

    Log::trace("< receiveDataFromCGI");
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
            try
            {
                Log::trace(">>> write sequence");
                if (this->isCGIPipe(fd))
                    sendDataToCGI(fd);
                else
                {
                    std::string response_message = this->makeResponseMessage(fd);
                    // std::cout << "message: " << response_message << std::endl;
                    // response_message = this->makeResponseMessage(this->_requests[fd], fd);
                    // TODO: sendResponse error handling
                    if (!(sendResponse(response_message, fd)))
                        std::cerr<<"Error: sendResponse"<<std::endl;
                    this->_server_manager->fdClr(fd, FdSet::WRITE);
                    this->_requests[fd].init();
                    this->_responses[fd].init();
                }
                Log::trace("<<< write sequence");
            }
            catch(const SendErrorCodeToClientException& e)
            {
                int client_fd = this->_server_manager->getLinkedFdFromFdTable(fd);
                this->closeFdAndSetFd(fd, FdSet::WRITE, client_fd, FdSet::WRITE);
                this->closeFdAndSetFd(this->_responses[client_fd].getReadFdFromCGI(), FdSet::READ, client_fd, FdSet::WRITE);
            }
            catch(const std::exception& e)
            {
                std::cerr << e.what() << '\n';
            }
        }
        else if (this->_server_manager->fdIsSet(fd, FdSet::READ))
        {
            try
            {
                std::cout << "Fd: " << fd << "FdType: " << Log::fdTypeToString(this->_server_manager->getFdType(fd)) << std::endl;
                if (this->isCGIPipe(fd)) 
                    receiveDataFromCGI(fd);
                else if (this->isStaticResource(fd))
                    this->readStaticResource(fd);
                else if (this->isClientSocket(fd))
                {
                    this->receiveRequest(fd);
                    Log::getRequest(*this, fd);
                    if (this->_requests[fd].getReqInfo() == ReqInfo::COMPLETE)
                        processResponseBody(fd);
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
    this->_requests[fd].init();
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
//NOTE: 설정 파일에서는 auth_basic_user_file file_paht; 로 있음
//NOTE: Server Generator에서 메인 반복문을 시작하기 전에
//NOTE: 설정파일의 auth_basic_user_file을 읽고 미리 decode 해놓자
//NOTE: 해당 value를 userid:password로 미리 세팅해주자
void
Server::checkAuthenticate(int fd) //NOTE: fd: client_fd
{
    std::string before_decode;
    std::string after_decode;
    Response& response = this->_responses[fd];
    Request& request = this->_requests[fd];
    const std::string& route = response.getRoute();
    const std::map<std::string, location_info>& location_config = this->getLocationConfig();
    const location_info& location_info = location_config.at(route);
    location_info::const_iterator it = location_info.find("auth_basic");
    if (it == location_info.end())
        return ;
    it = location_info.find("auth_basic_user_file");
    if (it == location_info.end())
        return ;
    const std::string& id_password = it->second;

    const std::map<std::string, std::string>& headers = this->_requests[fd].getHeaders();
    it = headers.find("Authorization");
    if (it == headers.end())
        throw (AuthenticateErrorException(this->_responses[fd], "401"));
    std::vector<std::string> authenticate_info = ft::split(it->second, " ");
    if (authenticate_info[0] != "Basic") //NOTE: 보안은 Basic 이용
        throw (AuthenticateErrorException(this->_responses[fd], "401"));
    before_decode = authenticate_info[1];
    Base64::decode(before_decode, after_decode);
    if (id_password != after_decode)
        throw (AuthenticateErrorException(this->_responses[fd], "403"));
    request.setAuthType(authenticate_info[0]);
    size_t pos = after_decode.find(":");
    request.setRemoteUser(after_decode.substr(0, pos));
    request.setRemoteIdent(after_decode.substr(pos + 1));
    std::cout << after_decode << std::endl;
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
    response.setUriPath(path);
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
    Log::trace("> readStaticResource");
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
        Log::openFd(*this, fd, FdType::RESOURCE, resource_fd);
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
    if (this->isCGIUri(fd, response.getUriExtension()))
    {
        response.setResourceType(ResType::CGI);
        return ;
    }
    DIR* dir_ptr;
    if ((dir_ptr = opendir(response.getResourceAbsPath().c_str())) != NULL)
    {
        response.setDirectoryEntry(dir_ptr);
        closedir(dir_ptr);
        if (this->isIndexFileExist(fd))
            response.setResourceType(ResType::INDEX_HTML);
        else
        {
            if (this->isAutoIndexOn(fd))
                response.setResourceType(ResType::AUTO_INDEX);
            else
                throw (IndexNoExistException(this->_responses[fd]));
        }
    }
    else
    {
        if (errno == ENOTDIR)
            response.setResourceType(ResType::STATIC_RESOURCE);
        else if (errno == EACCES)
            throw (CannotOpenDirectoryException(this->_responses[fd], "403", errno));
        else if (errno == ENOENT)
            throw (CannotOpenDirectoryException(this->_responses[fd], "404", errno));
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
        this->_server_manager->fdSet(fd, FdSet::WRITE);
        break ;
    case ResType::STATIC_RESOURCE:
        std::cout << "Static resource will be opened" << std::endl;
        this->openStaticResource(fd);
        break ;
    case ResType::CGI:
        std::cout << "CGIpipe will be opened" << std::endl;
        this->openCGIPipe(fd); //fd: client fd, cgiPipe[2]
        this->forkAndExecuteCGI(fd);
        // write flag -> CGI 프로세스에 스탠다드 인(pipe[1])으로 데이터를 넣어줌.
        // Client fd에 대해서 clear 해준다.
        break ;
    default:
        this->_server_manager->fdSet(fd, FdSet::WRITE);
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
    checkAuthenticate(fd);
    if (this->_responses[fd].isLocationToBeRedirected())
        throw (MustRedirectException(this->_responses[fd]));

    this->checkAndSetResourceType(fd);
    if (this->_responses[fd].getResourceType() == ResType::INDEX_HTML)
        this->setResourceAbsPathAsIndex(fd);
    ResType res_type = this->_responses[fd].getResourceType();
    preprocessResponseBody(fd, res_type);

    Log::trace("< processResopnseBody");
}

void
Server::openCGIPipe(int client_fd)
{
    Log::trace("> openCGIPipe");
    Response& response = this->_responses[client_fd];
    int pipe1[2];
    int pipe2[2];

    //TODO: 예외객체
    if (pipe(pipe1) < 0)
        throw (CgiExecuteErrorException(this->_responses[client_fd]));
    if (pipe(pipe2) < 0)
        throw (CgiExecuteErrorException(this->_responses[client_fd]));

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
    Log::trace("< openCGIPipe");
}

char**
Server::makeCGIEnvp(int client_fd)
{
    Log::trace("> makeCGIEnvp");
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
        if (!(envp[0] = ft::strdup("AUTH_TYPE=" + this->_requests[client_fd].getAuthType())))
            return (nullptr);
    }
    // TODO: Authorization있을때만 해야하는 지 확인할 것
    // REMOTE_USER; 1
    if (!(envp[1] = ft::strdup("REMOTE_USER=" + this->_requests[client_fd].getRemoteUser())))
        return (nullptr);
    // TODO: Authorization있을때만 해야하는 지 확인할 것
    // REMOTE_IDENT 2
    if (!(envp[2] = ft::strdup("REMOTE_IDENT=" + this->_requests[client_fd].getRemoteIdent())))
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
    if (!(envp[6] = ft::strdup("PATH_INFO=" + this->_responses[client_fd].getUriPath())))
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
        if (!(envp[11] = ft::strdup("REQUEST_URI=" + this->_responses[client_fd].getUriPath())))
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
    Log::trace("< makeCGIEnvp");
    return (envp);
}

char**
Server::makeCGIArgv(int client_fd)
{
    Log::trace("> makeCGIArgv");
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
    Log::trace("< makeCGIArgv");
    return (argv);
}

void
Server::forkAndExecuteCGI(int client_fd)
{
    Log::trace("> forkAndExecuteCGI");

    Response& response = this->_responses[client_fd];
    // Request& request = this->_requests[client_fd];
    int stdin_of_cgi = response.getStdinOfCGI();
    int stdout_of_cgi = response.getStdoutOfCGI();
    //TODO: 만약 아래의 두 함수가 return False를 한다면 할당 해놓은 자원 모두를 해체하고 throw 500을 하자.
    char** argv = this->makeCGIArgv(client_fd);
    char** envp = this->makeCGIEnvp(client_fd);
    if (argv == nullptr || envp == nullptr)
        throw (CgiExecuteErrorException(this->_responses[client_fd]));
    pid_t pid;
    int ret;

    //TODO: CGI Exception 만들기
    if ((pid = fork()) < 0)
        throw (CgiExecuteErrorException(this->_responses[client_fd]));
    else if (pid == 0)
    {
        close(response.getWriteFdToCGI());
        close(response.getReadFdFromCGI());
        if (dup2(stdin_of_cgi, 0) < 0)
            throw (CgiExecuteErrorException(this->_responses[client_fd]));
        if (dup2(stdout_of_cgi, 1) < 0)
            throw (CgiExecuteErrorException(this->_responses[client_fd]));
        //TODO: execve실패했을 경우 생각해보기
        if ((ret = execve(argv[0], argv, envp)) < 0)
            exit(ret);
        exit(ret);
    }
    else
    {
        close(stdin_of_cgi);
        close(stdout_of_cgi);
        response.setCGIPid(pid);
        ft::doubleFree(&argv);
        ft::doubleFree(&envp);
        // NOTE 정상적으로 읽으면 select 알아서 clear 해준다.
        this->_server_manager->fdSet(response.getWriteFdToCGI(), FdSet::WRITE);
    }
    Log::trace("< forkAndExecuteCGI");
}
