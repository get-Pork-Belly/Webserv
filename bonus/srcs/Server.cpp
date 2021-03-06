#include "Server.hpp"
#include "utils.hpp"
#include "ServerManager.hpp"
#include "Log.hpp"
#include "UriParser.hpp"
#include "Base64.hpp"
#include <iostream>
#include <signal.h>
#include <time.h>

/*============================================================================*/
/****************************  Static variables  ******************************/
/*============================================================================*/

extern std::vector<int> g_child_process_ids;

/*============================================================================*/
/******************************  Constructor  *********************************/
/*============================================================================*/

Server::Server(ServerManager* server_manager, server_info& server_config, std::map<std::string, location_info>& location_config)
: _server_manager(server_manager), _server_config(server_config),
_server_socket(-1), _host(server_config["server_name"]), _port(""),
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

Response&
Server::getResponse(int fd)
{
    return (this->_responses[fd]);
}

const std::map<std::string, std::vector<std::string> >&
Server::getAuthenticateRealms() const
{
    return (this->_authenticate_realms);
}

/*============================================================================*/
/********************************  Setter  ************************************/
/*============================================================================*/

void
Server::setResourceAbsPathAsIndex(int fd)
{
    Response& response = this->_responses[fd];
    const location_info& location_info = response.getLocationInfo();
    std::vector<std::string> indexs = ft::split(location_info.at("index"), " ");

    const std::string& path = response.getResourceAbsPath();
    for (std::string& index : indexs)
    {
        if (response.isFileInDirEntry(index))
        {
            response.setResourceType(ResType::STATIC_RESOURCE);
            if (path.back() == '/')
                response.setResourceAbsPath(path + index);
            else
                response.setResourceAbsPath(path + "/" + index);
        }
    }
}

void
Server::setAuthBasic(const std::string& auth_basic, const std::string& route)
{
    this->_location_config[route]["auth_basic"] = auth_basic;
}

void
Server::setAuthBasicUserFile(const std::string& decoded_id_password, const std::string& route)
{
    this->_location_config[route]["auth_basic_user_file"] = decoded_id_password;
}

void
Server::setAuthenticateRealms()
{
    int fd;
    int bytes;
    char temp[BUFFER_SIZE + 1];
    std::string readed;
    std::vector<std::string> auths;
    std::string after_decode;

    const std::map<std::string, location_info>& locations = this->getLocationConfig();
    for (auto& location: locations)
    {
        const location_info& location_info = location.second;
        if (location_info.at("auth_basic") != "off")
        {
            const location_info::const_iterator& it = location_info.find("auth_basic_user_file");
            if (it != location_info.end())
            {
                fd = open(it->second.c_str(), O_RDONLY, 0644);
                if (fd < 0)
                    setAuthBasic("off", location.first);
                else
                {
                    const std::string& route = location.first;
                    while ((bytes = read(fd, temp, BUFFER_SIZE)) > 0)
                    {
                        temp[bytes] = 0;
                        readed.append(temp, bytes);
                    }
                    close(fd);
                    if (bytes == -1)
                        this->setAuthBasic("off", route);
                    auths = ft::split(readed, "\n");
                    for (auto& auth : auths)
                    {
                        Base64::decode(auth, after_decode);
                        this->_authenticate_realms[route].push_back(after_decode);
                    }
                }
            }
        }
    }
}

/*============================================================================*/
/******************************  Exception  ***********************************/
/*============================================================================*/
Server::PayloadTooLargeException::PayloadTooLargeException(Server& server, int client_fd)
{
    server._server_manager->fdSet(client_fd, FdSet::WRITE);
    server._responses[client_fd].setStatusCode("413");
}

const char*
Server::PayloadTooLargeException::what() const throw()
{
    return ("[CODE 413] Payload Too Large");
}

Server::ReadErrorException::ReadErrorException(Server& server, int client_fd)
{
    server._server_manager->fdSet(client_fd, FdSet::WRITE);
    server._responses[client_fd].setStatusCode("500");
}

const char*
Server::ReadErrorException::what() const throw()
{
    return ("[CODE 500] Read empty buffer or occured reading error");
}

Server::MustRedirectException::MustRedirectException(Server& server, int client_fd)
: _msg("MustRedirectException: [CODE " + server._responses[client_fd].getRedirectStatusCode() + "]")
{
    server._server_manager->fdSet(client_fd, FdSet::WRITE);
    Response& response = server.getResponse(client_fd);
    response.setStatusCode(response.getRedirectStatusCode());
}

const char*
Server::MustRedirectException::what() const throw()
{
    return (this->_msg.c_str());
}

Server::CannotOpenDirectoryException::CannotOpenDirectoryException(Server& server, int client_fd, const std::string& status_code, int error_num)
:_msg("CannotOpenDirectoryException: " + std::string(strerror(error_num)))
{
    server._server_manager->fdSet(client_fd, FdSet::WRITE);
    server._responses[client_fd].setStatusCode(status_code);
}

const char*
Server::CannotOpenDirectoryException::what() const throw()
{
    return (this->_msg.c_str());
}

Server::OpenResourceErrorException::OpenResourceErrorException(Server& server, int client_fd, int error_num)
:_msg("OpenResourceErrorException: " + std::string(strerror(error_num)))
{
    Response& response = server.getResponse(client_fd);
    server._server_manager->fdSet(client_fd, FdSet::WRITE);
    if (error_num == EACCES)
        response.setStatusCode("403");
    else if (error_num == ENOMEM)
        response.setStatusCode("500");
    else
        response.setStatusCode("404");
}

const char*
Server::OpenResourceErrorException::what() const throw()
{
    return (this->_msg.c_str());
}

Server::IndexNoExistException::IndexNoExistException(Server& server, int client_fd)
{
    server._server_manager->fdSet(client_fd, FdSet::WRITE);
    server._responses[client_fd].setStatusCode("404");
}

const char*
Server::IndexNoExistException::what() const throw()
{
    return ("[CODE 404] No index & Autoindex off");
}

Server::CgiMethodErrorException::CgiMethodErrorException(Server& server, int client_fd)
{
    server._server_manager->fdSet(client_fd, FdSet::WRITE);
    server._responses[client_fd].setStatusCode("400");
}

const char*
Server::CgiMethodErrorException::what() const throw()
{
    return ("[CODE 400] CGI can handle only GET HEAD POST");
}

Server::InternalServerException::InternalServerException(Server& server, int client_fd)
{
    server._server_manager->fdSet(client_fd, FdSet::WRITE);
    server._responses[client_fd].setStatusCode("500");
}

const char*
Server::InternalServerException::what() const throw()
{
    return ("[CODE 500] Server Internal error");
}

Server::AuthenticateErrorException::AuthenticateErrorException(Server& server, int client_fd, const std::string& status_code)
{
    server._server_manager->fdSet(client_fd, FdSet::WRITE);
    server._responses[client_fd].setStatusCode(status_code);
}

const char*
Server::AuthenticateErrorException::what() const throw()
{
    return ("Authenticate Error Exception");
}

Server::CannotPutOnDirectoryException::CannotPutOnDirectoryException(Response& response)
: _response(response)
{
    this->_response.setStatusCode("409");
}

const char*
Server::CannotPutOnDirectoryException::what() const throw()
{
    return ("[CODE 409] Cannot put on directory exception");
}

Server::TargetResourceConflictException::TargetResourceConflictException(Server& server, int client_fd)
{
    server._server_manager->fdSet(client_fd, FdSet::WRITE);
    server._responses[client_fd].setStatusCode("409");
}

const char*
Server::TargetResourceConflictException::what() const throw()
{
    return ("[CODE 409] Target resource conflict exception");
}

Server::UnchunkedErrorException::UnchunkedErrorException(Server& server, int client_fd)
{
    server._responses[client_fd].setStatusCode("500");
    server._server_manager->fdSet(client_fd, FdSet::WRITE);
}

const char*
Server::UnchunkedErrorException::what() const throw()
{
    return ("[CODE 500] Chunked request couldn't receive or Receive error");
}

Server::NotAllowedMethodException::NotAllowedMethodException(Server& server, int client_fd)
{
    server._responses[client_fd].setStatusCode("405");
    server._server_manager->fdSet(client_fd, FdSet::WRITE);
}

const char*
Server::NotAllowedMethodException::what() const throw()
{
    return ("[CODE 405] Not Allowed Method");
}

Server::ReadStaticResourceErrorException::ReadStaticResourceErrorException(Server& server, int resource_fd)
{
    int client_fd = server._server_manager->getLinkedFdFromFdTable(resource_fd);
    server._server_manager->closeStaticResource(server, resource_fd);
    server._responses[client_fd].setStatusCode("500");
    server._server_manager->fdSet(client_fd, FdSet::WRITE);
}

const char*
Server::ReadStaticResourceErrorException::what() const throw()
{
    return ("[CODE 500] Read Static Resource exception");
}

Server::CannotWriteToClientException::CannotWriteToClientException(Server& server, int client_fd)
{
    server.closeClientSocket(client_fd);
    server._responses[client_fd].init();
    server._requests[client_fd].init();
}

const char*
Server::CannotWriteToClientException::what() const throw()
{
    return ("[CODE 901] Cannot write to client");
}

Server::ReceiveDataFromCgiPipeErrorException::ReceiveDataFromCgiPipeErrorException(Server& server, int read_fd_from_cgi)
{
    int client_fd = server._server_manager->getLinkedFdFromFdTable(read_fd_from_cgi);
    int write_fd_to_cgi = server._responses[client_fd].getWriteFdToCgi();

    kill(server._responses[client_fd].getCgiPid(), SIGTERM);
    server._server_manager->closeCgiWritePipe(server, write_fd_to_cgi);
    server._server_manager->closeCgiReadPipe(server, read_fd_from_cgi);
    server._responses[client_fd].setStatusCode("500");
    server._server_manager->fdSet(client_fd, FdSet::WRITE);
}

const char*
Server::ReceiveDataFromCgiPipeErrorException::what() const throw()
{
    return ("[CODE 500] Receive data from cgi error exception");
}

Server::SendDataToCgiPipeErrorException::SendDataToCgiPipeErrorException(Server& server, int write_fd_to_cgi)
{
    int client_fd = server._server_manager->getLinkedFdFromFdTable(write_fd_to_cgi);
    int read_fd_from_cgi= server._responses[client_fd].getReadFdFromCgi();

    kill(server._responses[client_fd].getCgiPid(), SIGTERM);
    server._server_manager->closeCgiWritePipe(server, write_fd_to_cgi);
    server._server_manager->closeCgiReadPipe(server, read_fd_from_cgi);
    server._responses[client_fd].setStatusCode("500");
    server._server_manager->fdSet(client_fd, FdSet::WRITE);
}

const char*
Server::SendDataToCgiPipeErrorException::what() const throw()
{
    return ("[CODE 500] Send data to cgi error exception");
}

Server::PutFileOnServerErrorException::PutFileOnServerErrorException(Server& server, int resource_fd)
{
    int client_fd = server._server_manager->getLinkedFdFromFdTable(resource_fd);
    server._server_manager->closeStaticResource(server, resource_fd);
    server._responses[client_fd].setStatusCode("500");
    server._server_manager->fdSet(client_fd, FdSet::WRITE);
}

const char*
Server::PutFileOnServerErrorException::what() const throw()
{
    return ("[CODE 500] Put file on server error exception");
}

/*============================================================================*/
/*********************************  Util  *************************************/ 
/*============================================================================*/

std::string
Server::makeDefaultErrorPage()
{
    char buf[BUFFER_SIZE + 1];
    std::string default_error_template = "<html>\n\t<head>\n\t\t<title>";
    default_error_template += "<- status_code -> <- error_message ->";
    default_error_template += "</title>\n\t</head>";
    default_error_template += "\n\t<body>\n\t\t<center>\n\t\t\t<h1>";
    default_error_template += "<- status_code -> <- error_message ->";
    default_error_template += "</h1>\n\t\t</center>";
    default_error_template += "\n\t\t<hr>\n\t<center> ft_nginx </center>\n\t</body>\n</html>";

    std::string default_error_page = "";
    std::string error_page_path;
    if (this->_server_config.find("default_error_page") != this->_server_config.end())
    {
        error_page_path = this->_server_config["default_error_page"];

        int fd = open(error_page_path.c_str(), O_RDONLY);
        if (fd > 0)
        {
            default_error_page = "";
            int readed;
            while ((readed = read(fd, buf, BUFFER_SIZE)) > 0)
            {
                buf[readed] = 0;
                default_error_page += std::string(buf, readed);
            }
            if (readed == -1)
                default_error_page = default_error_template;
        }
    }
    else
        default_error_page = default_error_template;
    return default_error_page;
}

//TODO: take care init errors
void
Server::init()
{
    for (auto& conf: this->_server_config)
    {
        if (conf.first == "server_name")
            this->_host = conf.second;
        else if (conf.first == "listen")
            this->_port = conf.second;
    }
    this->_requests = std::vector<Request>(1024);
    this->_responses = std::vector<Response>(1024);

    std::string default_error_page = this->makeDefaultErrorPage();
    for(auto& response : this->_responses)
        response.setErrorPage(default_error_page);
    this->setAuthenticateRealms();

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

bool
Server::isExistCrlf(int fd, const RecvRequest recv_request)
{
    if (recv_request == RecvRequest::REQUEST_LINE)
        return (this->_requests[fd].getIndexOfCrlfInRequestLine() != DEFAULT_INDEX_OF_CRLF);
    return (this->_requests[fd].getIndexOfCrlfInChunkSize() != DEFAULT_INDEX_OF_CRLF);
}

bool
Server::isNotYetSetTargetChunkSize(int fd)
{
    return (this->_requests[fd].getTargetChunkSize() == DEFAULT_TARGET_CHUNK_SIZE);
}

void
Server::findCrlfAndSetIndexOfCrlf(int fd, const std::string& buf, const RecvRequest recv_request)
{
    size_t index_of_crlf;

    if ((index_of_crlf = buf.find("\r\n")) != std::string::npos)
    {
        if (recv_request == RecvRequest::REQUEST_LINE)
            this->_requests[fd].setIndexOfCrlfInRequestLine(index_of_crlf);
        else if (recv_request == RecvRequest::CHUNKED_BODY)
            this->_requests[fd].setIndexOfCrlfInChunkSize(index_of_crlf);
    }
    else
    {
        if (buf.length() >= BUFFER_SIZE - 2)
            throw (Request::UriTooLongException(this->_requests[fd]));
        else if (recv_request == RecvRequest::CHUNKED_BODY)
            throw (Request::RequestFormatException(this->_requests[fd], "400"));
    }
}

bool
Server::isLastSequenceOfParsingChunk(int fd)
{
    return (this->_requests[fd].getTargetChunkSize() == 0);
}

int
Server::calculateReceiveTargetSizeOfChunkData(int fd)
{
    return (std::min(RECEIVE_SOCKET_STREAM_SIZE,
            this->_requests[fd].getTargetChunkSize() + CRLF_SIZE - this->_requests[fd].getReceivedChunkDataLength()));
}

bool
Server::isChunkDataAllReceived(int fd)
{
    return ((this->_requests[fd].getTargetChunkSize() + CRLF_SIZE == this->_requests[fd].getReceivedChunkDataLength()));
}

void
Server::prepareToReceiveNextChunkSize(int fd)
{
    this->_requests[fd].setReceivedChunkDataLength(0);
    this->_requests[fd].setTargetChunkSize(DEFAULT_TARGET_CHUNK_SIZE);
}

void
Server::prepareToReceiveNextChunkData(int fd)
{
    this->_requests[fd].setIndexOfCrlfInChunkSize(DEFAULT_INDEX_OF_CRLF);
    this->_requests[fd].setTempBuffer("");
    this->_requests[fd].setReceivedChunkSizeLength(0);
}

void
Server::prepareToReceiveHeaders(int fd)
{
    this->_requests[fd].setIndexOfCrlfInRequestLine(DEFAULT_INDEX_OF_CRLF);
    this->_requests[fd].setTempBuffer("");
    this->_requests[fd].setReceivedRequestLineLength(0);
    this->_requests[fd].setRecvRequest(RecvRequest::HEADERS);
}

void
Server::finishChunkSequence(int fd)
{
    this->_requests[fd].setRecvRequest(RecvRequest::COMPLETE);
    this->_requests[fd].setTempBuffer("");
    this->_requests[fd].setReceivedLastChunkDataLength(0);
    this->_requests[fd].setTargetChunkSize(DEFAULT_TARGET_CHUNK_SIZE);
}

void
Server::readBufferUntilRequestLine(int client_fd)
{

    Log::trace("> readBufferUntilRequestLine", 1);
    timeval from;
    gettimeofday(&from, NULL);

    int bytes;
    char buf[BUFFER_SIZE + 1];
    Request& request = this->_requests[client_fd];

    int index_of_crlf = request.getIndexOfCrlfInRequestLine();
    int received_request_line_length = request.getReceivedRequestLineLength();

    if ((bytes = read(client_fd, buf, index_of_crlf + CRLF_SIZE)) > 0)
    {
        buf[bytes] = 0;
        received_request_line_length += bytes;
        request.setReceivedRequestLineLength(received_request_line_length);

        if (received_request_line_length == static_cast<int>(index_of_crlf) + CRLF_SIZE)
        {
            request.appendTempBuffer(buf, bytes);
            request.parseRequestLine();
            this->prepareToReceiveHeaders(client_fd);
        }
        else
            request.appendTempBuffer(buf, bytes);
    }
    else if (bytes == 0)
        throw (Request::RequestFormatException(request, "400"));
    else
        throw (ReadErrorException(*this, client_fd));

    Log::printTimeDiff(from, 1);
    Log::trace("< readBufferUntilRequestLine", 1);
}

bool
Server::readBufferUntilHeaders(int client_fd, char* buf, size_t read_target_size)
{
    Log::trace("> readBufferUntilHeaders", 1);
    timeval from;
    gettimeofday(&from, NULL);

    int bytes;
    Request& request = this->_requests[client_fd];

    if ((bytes = read(client_fd, buf, read_target_size)) > 0)
    {
        buf[bytes] = 0;
        request.appendTempBuffer(buf, bytes);
        return (static_cast<size_t>(bytes) == read_target_size);
    }
    else if (bytes == 0)
        throw (Request::RequestFormatException(request, "400"));
    else
        throw (ReadErrorException(*this, client_fd));
    return (true);

    Log::printTimeDiff(from, 1);
    Log::trace("< readBufferUntilHeaders", 1);
}

void
Server::processIfHeadersNotFound(int client_fd, const std::string& peeked_message)
{
    size_t bytes;
    char buf[BUFFER_SIZE + 1];

    if (peeked_message == "\r\n")
    {
        bytes = read(client_fd, buf, CRLF_SIZE);
        if (bytes > 0)
        {
            buf[bytes] = 0;
            if (bytes != CRLF_SIZE)
                throw (ReadErrorException(*this, client_fd));
        }
        else if (bytes == 0)
            throw (ReadErrorException(*this, client_fd));
        else
            throw (ReadErrorException(*this, client_fd));
    }
}

void
Server::findAndExecuteEchoPlugin(int client_fd)
{
    Response& response = this->_responses[client_fd];
    const location_info& locations = response.getLocationInfo();

    if (locations.find("location_msg") != locations.end() &&
            this->_server_manager->isPluginOn("show_location_msg"))
    {
        std::string echo_string = "echo " + locations.at("location_msg");
        system(echo_string.c_str());
    }
}

void
Server::receiveRequestLine(int client_fd)
{
    Log::trace("> receiveRequestLine", 1);
    timeval from;
    gettimeofday(&from, NULL);

    int bytes;
    char buf[BUFFER_SIZE + 1];
    Request& request = this->_requests[client_fd];

    if ((bytes = request.peekMessageFromClient(client_fd, buf)) > 0)
    {
        buf[bytes] = 0;
        if (this->isExistCrlf(client_fd, RecvRequest::REQUEST_LINE))
        {
            this->readBufferUntilRequestLine(client_fd);
            this->parseUriAndSetResponse(client_fd);
            this->findAndExecuteEchoPlugin(client_fd);
        }
        else
            this->findCrlfAndSetIndexOfCrlf(client_fd, buf, RecvRequest::REQUEST_LINE);
    }
    else if (bytes == RECV_COUNT_NOT_REACHED)
        request.raiseRecvCounts();
    else if (bytes == 0)
        this->closeClientSocket(client_fd);
    else
        throw (ReadErrorException(*this, client_fd));

    Log::printTimeDiff(from, 1);
    Log::trace("< receiveRequestLine", 1);
}

void
Server::receiveRequestHeaders(int client_fd)
{
    Log::trace("> receiveRequestHeaders", 1);
    timeval from;
    gettimeofday(&from, NULL);

    char buf[BUFFER_SIZE + 1];
    int peeked_bytes;
    Request& request = this->_requests[client_fd];
    if ((peeked_bytes = request.peekMessageFromClient(client_fd, buf)) > 0)
    {
        buf[peeked_bytes] = 0;
        int read_target_size = request.calculateReadTargetSize(buf, peeked_bytes);
        if (read_target_size == SHOULD_RECEIVE_MORE)
            this->readBufferUntilHeaders(client_fd, buf, BUFFER_SIZE);
        else
        {
            if (this->readBufferUntilHeaders(client_fd, buf, read_target_size))
            {
                request.parseRequestHeaders();
                if (peeked_bytes == read_target_size)
                {
                    if (request.isBodyUnnecessary() ||
                        (request.isNormalBody() && request.getHeaders().at("Content-Length") == "0"))
                        request.setRecvRequest(RecvRequest::COMPLETE);
                }
                request.updateRecvRequest();
                this->checkAuthenticate(client_fd);
            }
        }
    }
    else if (peeked_bytes == RECV_COUNT_NOT_REACHED)
        request.raiseRecvCounts();
    else if (peeked_bytes == 0)
        this->closeClientSocket(client_fd);
    else
        throw (ReadErrorException(*this, client_fd));

    
    Log::printTimeDiff(from, 1);
    Log::trace("< receiveRequestHeaders", 1);
}

void
Server::receiveRequestNormalBody(int client_fd)
{
    Log::trace("> receiveRequestNormalBody", 1);
    timeval from;
    gettimeofday(&from, NULL);

    int bytes;
    Request& request = this->_requests[client_fd];

    int content_length = request.getContentLength();
    if (content_length > DEFAULT_LIMIT_CLIENT_BODY_LENGTH)
        throw (PayloadTooLargeException(*this, client_fd));
    char buf[BUFFER_SIZE + 1];
    if ((bytes = recv(client_fd, buf, BUFFER_SIZE, 0)) > 0)
    {
        buf[bytes] = 0;
        request.appendBody(buf, bytes);
        if (request.getBody().length() < static_cast<size_t>(content_length))
            return ;
        else if (request.getBody().length() == static_cast<size_t>(content_length))
            request.setRecvRequest(RecvRequest::COMPLETE);
        else
            throw (PayloadTooLargeException(*this, client_fd));
        if (bytes < BUFFER_SIZE)
            request.setRecvRequest(RecvRequest::COMPLETE);
    }
    else if (bytes == 0)
        this->closeClientSocket(client_fd);
    else
        throw (ReadErrorException(*this, client_fd));

    Log::printTimeDiff(from, 1);
    Log::trace("< receiveRequestNormalBody", 1);
}

void
Server::receiveChunkSize(int client_fd)
{
    Log::trace("> receiveChunkSize", 1);
    timeval from;
    gettimeofday(&from, NULL);

    int bytes;
    char buf[RECEIVE_SOCKET_STREAM_SIZE + 1];
    Request& request = this->_requests[client_fd];

    int index_of_crlf = request.getIndexOfCrlfInChunkSize();
    int received_chunk_size_length = request.getReceivedChunkSizeLength();

    if ((bytes = recv(client_fd, buf, index_of_crlf + CRLF_SIZE, 0)) > 0)
    {
        buf[bytes] = 0;
        received_chunk_size_length += bytes;
        request.setReceivedChunkSizeLength(received_chunk_size_length);

        if (received_chunk_size_length == static_cast<int>(index_of_crlf) + CRLF_SIZE)
        {
            request.appendTempBuffer(buf, bytes);
            request.parseTargetChunkSize(request.getTempBuffer());
            this->prepareToReceiveNextChunkData(client_fd);
        }
        else
            request.appendTempBuffer(buf, bytes);
    }
    else if (bytes == 0)
        this->closeClientSocket(client_fd);
    else
        throw (UnchunkedErrorException(*this, client_fd));

    Log::printTimeDiff(from, 1);
    Log::trace("< receiveChunkSize", 1);
}

void
Server::receiveChunkData(int client_fd, int receive_size)
{

    Log::trace("> receiveChunkData", 1);
    timeval from;
    gettimeofday(&from, NULL);

    int bytes;
    char buf[RECEIVE_SOCKET_STREAM_SIZE + 1];
    Request& request = this->_requests[client_fd];

    if ((bytes = recv(client_fd, buf, receive_size, 0)) > 0)
    {
        buf[bytes] = 0;
        request.setReceivedChunkDataLength(request.getReceivedChunkDataLength() + bytes);
        request.parseChunkData(buf, bytes);
    }
    else if (bytes == 0)
        this->closeClientSocket(client_fd);
    else
        throw (UnchunkedErrorException(*this, client_fd));

    Log::printTimeDiff(from, 1);
    Log::trace("< receiveChunkData", 1);
}

void
Server::receiveLastChunkData(int client_fd)
{
    Log::trace("> receiveLastChunkData", 1);
    timeval from;
    gettimeofday(&from, NULL);

    int bytes = 0;
    int is_buffer_left = 0;
    char buf[RECEIVE_SOCKET_STREAM_SIZE + 1];
    Request& request = this->_requests[client_fd];

    int received_last_chunk_data_length = request.getReceivedLastChunkDataLength();

    bytes = recv(client_fd, buf, CRLF_SIZE + 1, 0);
    if (bytes > 0)
    {
        buf[bytes] = 0;
        received_last_chunk_data_length += bytes;
        request.setReceivedLastChunkDataLength(received_last_chunk_data_length);

        if (received_last_chunk_data_length < CRLF_SIZE)
            request.appendTempBuffer(buf, bytes);
        else if (received_last_chunk_data_length > CRLF_SIZE)
            throw (Request::RequestFormatException(request, "400"));
        else
        {
            if ((is_buffer_left = recv(client_fd, buf, 1, MSG_PEEK)) > 0)
                throw (Request::RequestFormatException(request, "400"));
            else if (is_buffer_left == 0)
                this->closeClientSocket(client_fd);
            else
            {
                request.appendTempBuffer(buf, bytes);
                if (request.getTempBuffer().compare("\r\n") == 0)
                    this->finishChunkSequence(client_fd);
                else
                    throw (Request::RequestFormatException(request, "400"));
            }
        }
    }
    else if (bytes == 0)
        this->closeClientSocket(client_fd);
    else
        throw (UnchunkedErrorException(*this, client_fd));

    Log::printTimeDiff(from, 1);
    Log::trace("< receiveLastChunkData", 1);
}

void
Server::receiveRequestChunkedBody(int client_fd)
{
    Log::trace("> receiveRequestChunkedBody", 1);
    timeval from;
    gettimeofday(&from, NULL);

    int bytes;
    char buf[RECEIVE_SOCKET_STREAM_SIZE + 1];
    Request& request = this->_requests[client_fd];

    if ((bytes = request.peekMessageFromClient(client_fd, buf)) > 0)
    {
        buf[bytes] = 0;
        if (this->isExistCrlf(client_fd, RecvRequest::CHUNKED_BODY))
            this->receiveChunkSize(client_fd);
        else if (this->isNotYetSetTargetChunkSize(client_fd))
            this->findCrlfAndSetIndexOfCrlf(client_fd, buf, RecvRequest::CHUNKED_BODY);
        else if (this->isLastSequenceOfParsingChunk(client_fd))
            this->receiveLastChunkData(client_fd);
        else
        {
            int receive_target_size = this->calculateReceiveTargetSizeOfChunkData(client_fd);
            this->receiveChunkData(client_fd, receive_target_size);
            if (this->isChunkDataAllReceived(client_fd))
                this->prepareToReceiveNextChunkSize(client_fd);
        }
    }
    else if (bytes == RECV_COUNT_NOT_REACHED)
        request.raiseRecvCounts();
    else if (bytes == 0)
        this->closeClientSocket(client_fd);
    else
        throw (UnchunkedErrorException(*this, client_fd));
        
        
    Log::printTimeDiff(from, 1);
    Log::trace("< receiveRequestChunkedBody", 1);
}

void
Server::receiveRequest(int client_fd)
{
    Log::trace("> receiveRequest", 1);
    timeval from;
    gettimeofday(&from, NULL);

    const RecvRequest& req_info = this->_requests[client_fd].getRecvRequest();
    switch (req_info)
    {
    case RecvRequest::REQUEST_LINE:
        this->receiveRequestLine(client_fd);
        break ;

    case RecvRequest::HEADERS:
        this->receiveRequestHeaders(client_fd);
        //TODO: this->processResponseBody()
        break ;

    case RecvRequest::NORMAL_BODY:
        this->receiveRequestNormalBody(client_fd);
        break ;

    case RecvRequest::CHUNKED_BODY:
        this->receiveRequestChunkedBody(client_fd);
        break ;

    default:
        break ;
    }


    Log::printTimeDiff(from, 1);
    Log::trace("< receiveRequest", 1);
}

void
Server::setResponseMessageAccordingToTheParseProgress(int client_fd, std::string& status_line, std::string& headers)
{
    Request& request = this->_requests[client_fd];
    Response& response = this->_responses[client_fd];
    const std::string& method = request.getMethod();
    switch (response.getParseProgress())
    {
    case ParseProgress::DEFAULT:
        response.setParseProgress(ParseProgress::FINISH);
        if (method == "HEAD" || ((method == "PUT" || method == "DELETE") && response.getStatusCode().front() == '2'))
        {
            response.setResponseMessage(status_line + headers);
            break ;
        }
        response.setResponseMessage(status_line + headers + response.getTransmittingBody());
        break ;
    case ParseProgress::CHUNK_START:
        if (request.getMethod() == "HEAD")
        {
            response.setResponseMessage(status_line + headers);
            break ;
        }
        response.setResponseMessage(status_line + headers + response.getTransmittingBody());
        break ;
    default: //NOTE: PraseProgress::CHUNK_PROGRESS or ParseProgress::FINISH
        if (request.getMethod() == "HEAD")
        {
            response.setResponseMessage("");
            break ;
        }
        if (response.getResourceType() == ResType::CGI && response.getUriExtension() == ".py")
        {
            response.setResponseMessage(status_line + headers + response.getTransmittingBody());
            break ;
        }
        response.setResponseMessage(response.getTransmittingBody());
        break ;
    }
}

void
Server::makeResponseMessage(int client_fd)
{
    Log::trace("> makeResponseMessage", 1);
    timeval from;
    gettimeofday(&from, NULL);

    Request& request = this->_requests[client_fd];
    Response& response = this->_responses[client_fd];

    if (response.getStatusCode().front() != '2')
        response.setResourceType(ResType::ERROR_HTML);

    if (response.isRedirection(response.getStatusCode()) == false)
        response.makeBody(request);

    std::string status_line;
    std::string headers;
    if (response.getSendProgress() == SendProgress::READY)
    {
        headers = response.makeHeaders(request);
        status_line = response.makeStatusLine();
    }

    this->setResponseMessageAccordingToTheParseProgress(client_fd, status_line, headers);

    Log::printTimeDiff(from, 1);
    Log::trace("< makeResponseMessage", 1);

}

long long sended_bytes;

void
Server::sendResponse(int client_fd)
{
    Log::trace("> sendResponse", 1);
    timeval from;
    gettimeofday(&from, NULL);

    int bytes = 0;

    Response& response = this->_responses[client_fd];
    int sended_response_size = response.getSendedResponseSize();
    const std::string& response_message = response.getResponseMessage();
    int response_message_size = response_message.length();
    int remained = response_message_size - sended_response_size;

    bytes = write(client_fd, &response_message.c_str()[sended_response_size], remained);
    if (bytes > 0)
    {
        sended_bytes += bytes;
        sended_response_size += bytes;
        response.setSendedResponseSize(sended_response_size);
        if (sended_response_size == response_message_size)
        {
            response.setSendProgress(SendProgress::ALL_SENDED);
            response.setSendedResponseSize(0);
            response.setResponseMessage("");
        }
        else
            response.setSendProgress(SendProgress::SENDING);
    }
    else if (bytes == 0)
        throw (CannotWriteToClientException(*this, client_fd));
    else
        throw (CannotWriteToClientException(*this, client_fd));

    Log::printTimeDiff(from, 1);
    Log::trace("< sendResponse", 1);
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
    Log::trace("> isClientSocket", 2);
    timeval from;
    gettimeofday(&from, NULL);


    const std::vector<std::pair<FdType, int> >& fd_table = this->_server_manager->getFdTable();
    if (fd_table[fd].first == FdType::CLIENT_SOCKET)
    {
        Log::printTimeDiff(from, 2);
        Log::trace("< isClientSocket return true", 2);
        return true;
    }


    Log::printTimeDiff(from, 2);
    Log::trace("< isClientSocket return false", 2);
    return false;
}

bool
Server::isStaticResource(int fd) const
{
    Log::trace("> isStaticResource", 2);
    timeval from;
    gettimeofday(&from, NULL);


    const std::vector<std::pair<FdType, int> >& fd_table = this->_server_manager->getFdTable();
    if (fd_table[fd].first == FdType::RESOURCE)
    {
        Log::printTimeDiff(from, 2);
        Log::trace("< isStaticResource return true", 2);
        return true;
    }


    Log::printTimeDiff(from, 2);
    Log::trace("< isStaticResource return false", 2);
    return false;
}

bool
Server::isCgiPipe(int fd) const
{
    Log::trace("> isCgiPipe", 2);
    timeval from;
    gettimeofday(&from, NULL);


    const std::vector<std::pair<FdType, int> >& fd_table = this->_server_manager->getFdTable();
    if (fd_table[fd].first == FdType::PIPE)
    {
        Log::printTimeDiff(from, 2);
        Log::trace("< isCgiPipe return true", 2);
        return true;
    }


    Log::printTimeDiff(from, 2);
    Log::trace("< isCgiPipe return false", 2);
    return false;
}

bool
Server::isCgiWritePipe(int fd) const
{
    Log::trace("> isCgiWritePipe", 2);
    timeval from;
    gettimeofday(&from, NULL);


    const std::vector<std::pair<FdType, int> >& fd_table = this->_server_manager->getFdTable();
    if (fd_table[fd].first == FdType::PIPE && this->_responses[fd_table[fd].second].getWriteFdToCgi() == fd)
    {
        Log::printTimeDiff(from, 2);
        Log::trace("< isCgiWritePipe return true", 2);
        return true;
    }
    Log::printTimeDiff(from, 2);
    Log::trace("< isCgiWritePipe return false", 2);
    return false;
}

bool
Server::isCgiReadPipe(int fd) const
{
    Log::trace("> isCgiReadPipe", 2);
    timeval from;
    gettimeofday(&from, NULL);

    
    const std::vector<std::pair<FdType, int> >& fd_table = this->_server_manager->getFdTable();
    if (fd_table[fd].first == FdType::PIPE && this->_responses[fd_table[fd].second].getReadFdFromCgi() == fd)
    {
        Log::printTimeDiff(from, 2);
        Log::trace("< isCgiReadPipe return true", 2);
        return true;
    }


    Log::printTimeDiff(from, 2);
    Log::trace("< isCgiReadPipe return false", 2);
    return false;
}

bool
Server::isIndexFileExist(int client_fd)
{
    const location_info& location_info = this->_responses[client_fd].getLocationInfo();
    std::vector<std::string> indexs = ft::split(location_info.at("index"), " ");
    for (std::string& index : indexs)
    {
        if (this->_responses[client_fd].isFileInDirEntry(index))
            return (true);
    }
    return (false);
}

bool
Server::isAutoIndexOn(int client_fd)
{
    const location_info& location_info = this->_responses[client_fd].getLocationInfo();
    Log::printLocationInfo(location_info);
    if (location_info.at("autoindex") == "on")
        return (true);
    return (false);
}

bool
Server::isCgiUri(int client_fd, const std::string& extension)
{
    Log::trace("> isCgiUri", 2);
    timeval from;
    gettimeofday(&from, NULL);

    if (extension == "")
    {
        Log::printTimeDiff(from, 2);
        Log::trace("< isCgiUri return false", 2);
        return (false);
    }

    const location_info& location_info = this->_responses[client_fd].getLocationInfo();
    location_info::const_iterator it = location_info.find("cgi");
    if (it == location_info.end())
    {
        Log::printTimeDiff(from, 2);
        Log::trace("< isCgiUri return false", 2);
        return (false);
    }

    const std::string& cgi = it->second;
    if (cgi.find(extension) == std::string::npos)
    {
        Log::printTimeDiff(from, 2);
        Log::trace("< isCgiUri return false", 2);
        return (false);
    }

    Log::printTimeDiff(from, 2);
    Log::trace("< isCgiUri return true", 2);
    return (true);
}

void
Server::acceptClient()
{
    Log::trace("> acceptClient", 2);
    timeval from;
    gettimeofday(&from, NULL);

    
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

        
    Log::printTimeDiff(from, 2);
    Log::trace("< acceptClient", 2);
}

int transfered_bytes;

void
Server::sendDataToCgi(int write_fd_to_cgi)
{
    Log::trace("> sendDataToCgi", 1);
    timeval from;
    gettimeofday(&from, NULL);

    int client_fd = this->_server_manager->getLinkedFdFromFdTable(write_fd_to_cgi);
    Request& request = this->_requests[client_fd];
    Response& response = this->_responses[client_fd];

    const std::string& body = request.getBody();

    int transfered_body_size = request.getTransferedBodySize();

    int content_length;
    const std::map<std::string, std::string>& headers = request.getHeaders();
    if (headers.find("Content-Length") != headers.end())
        content_length = request.getContentLength();
    else
        content_length = body.length();

    int target_send_data_size = std::min(SEND_PIPE_STREAM_SIZE, content_length - transfered_body_size);

    int bytes;

    if ((bytes = write(write_fd_to_cgi, &body.c_str()[transfered_body_size], target_send_data_size)) > 0)
    {
        transfered_body_size += bytes;
        request.setTransferedBodySize(transfered_body_size);
        if (content_length == transfered_body_size)
            this->finishSendDataToCgiPipe(write_fd_to_cgi);
        else
        {
            this->_server_manager->fdSet(response.getReadFdFromCgi(), FdSet::READ);
            this->_server_manager->fdClr(write_fd_to_cgi, FdSet::WRITE);
        }
    }
    else if (bytes == 0)
        this->finishSendDataToCgiPipe(write_fd_to_cgi);
    else
        throw (SendDataToCgiPipeErrorException(*this, write_fd_to_cgi));

    Log::printTimeDiff(from, 1);
    Log::trace("< sendDataToCgi", 1);
}

long long received_bytes;

void
Server::receiveDataFromCgi(int read_fd_from_cgi)
{
    Log::trace("> receiveDataFromCgi", 1);
    timeval from;
    gettimeofday(&from, NULL);


    int bytes;
    int client_fd;

    client_fd = this->_server_manager->getLinkedFdFromFdTable(read_fd_from_cgi);
    Response& response = this->_responses[client_fd];

    char buf[BUFFER_SIZE + 1];
    bytes = read(read_fd_from_cgi, buf, BUFFER_SIZE);
    buf[bytes] = 0;

    if (bytes > 0)
    {
        buf[bytes] = 0;
        if (response.getReceiveProgress() == ReceiveProgress::CGI_BEGIN ||
            response.getReceiveProgress() == ReceiveProgress::PHP_CGI_BEGIN)
        {
            response.appendTempBuffer(buf, bytes);
            if (response.findEndOfHeaders())
            {
                if (response.getReceiveProgress() == ReceiveProgress::PHP_CGI_BEGIN)
                {
                    response.trimPhpCgiFirstHeadersFromTempBuffer();
                    response.setReceiveProgress(ReceiveProgress::CGI_BEGIN);
                }
                response.preparseCgiMessage();
                response.setReceiveProgress(ReceiveProgress::ON_GOING);
                this->_server_manager->fdClr(read_fd_from_cgi, FdSet::READ);
                this->_server_manager->fdSet(client_fd, FdSet::WRITE);
            }
            else
            {
                if (response.getTempBuffer().length() >= LIMIT_HEADERS_LENGTH)
                    throw (InternalServerException(*this, client_fd));
            }
        }
        else
        {
            response.appendBody(buf, bytes);
            this->_server_manager->fdClr(read_fd_from_cgi, FdSet::READ);
            this->_server_manager->fdSet(client_fd, FdSet::WRITE);
        }
    }
    else if (bytes == 0)
        this->finishReceiveDataFromCgiPipe(read_fd_from_cgi);
    else
        throw (ReceiveDataFromCgiPipeErrorException(*this, read_fd_from_cgi));

    Log::printTimeDiff(from, 1);
    Log::trace("< receiveDataFromCgi", 1);
}

void
Server::putFileOnServer(int resource_fd)
{
    int bytes;
    int client_fd = this->_server_manager->getLinkedFdFromFdTable(resource_fd);
    Request& request = this->_requests[client_fd];
    const std::string& body = request.getBody();

    int transfered_body_size = request.getTransferedBodySize();
    int remained = body.length() - transfered_body_size;
    if (BUFFER_SIZE > remained)
    { 
        bytes = write(resource_fd, &(body.c_str()[transfered_body_size]), remained);
        if (bytes > 0)
        {
            transfered_body_size += bytes;
            request.setTransferedBodySize(transfered_body_size);
            if (bytes == remained)
                this->finishPutFileOnServer(resource_fd);
        }
        else if (bytes == 0)
            this->finishPutFileOnServer(resource_fd);
        else
            throw (PutFileOnServerErrorException(*this, resource_fd));
    }
    else
    {
        bytes = write(resource_fd, &(body.c_str()[transfered_body_size]), BUFFER_SIZE);
        if (bytes > 0)
        {
            transfered_body_size += bytes;
            request.setTransferedBodySize(transfered_body_size);
        }
        else if (bytes == 0)
        {
            if (remained > 0)
                throw (PutFileOnServerErrorException(*this, resource_fd));
        }
        else
            throw (PutFileOnServerErrorException(*this, resource_fd));
    }
}

void
Server::writeSequence(int fd)
{
    timeval from;
    gettimeofday(&from, NULL);
    Log::trace(">>> write sequence", 1);

    if (this->isCgiWritePipe(fd))
        this->sendDataToCgi(fd);
    else if (this->isClientSocket(fd))
    {
        if (this->_responses[fd].getSendProgress() != SendProgress::SENDING)
            this->makeResponseMessage(fd);
        this->sendResponse(fd);
        if (this->_responses[fd].getReceiveProgress() == ReceiveProgress::ON_GOING)
        {
            this->_server_manager->fdClr(fd, FdSet::WRITE);
            if (this->_responses[fd].getResourceFd() != DEFAULT_FD)
                this->_server_manager->fdSet(this->_responses[fd].getResourceFd(), FdSet::READ);
            else
            {
                if (this->_responses[fd].getWriteFdToCgi() == DEFAULT_FD)
                    this->_server_manager->fdSet(this->_responses[fd].getReadFdFromCgi(), FdSet::READ);
                else
                    this->_server_manager->fdSet(this->_responses[fd].getWriteFdToCgi(), FdSet::WRITE);
            }
        }
        if (this->isResponseAllSended(fd))
        {
            if (this->_responses[fd].getStatusCode()[0] != '2')
                this->closeClientSocket(fd);
            else
            {
                this->_server_manager->fdClr(fd, FdSet::WRITE);
                this->_server_manager->fdSet(fd, FdSet::READ);
                this->_requests[fd].init();
                this->_responses[fd].init();
            }
            this->_server_manager->monitorTimeOutOff(fd);
        }
    }
    else
        this->putFileOnServer(fd);

    Log::printTimeDiff(from, 1);
    Log::trace("<<< write sequence", 1);
}

void
Server::readSequence(int fd)
{
    timeval from;
    gettimeofday(&from, NULL);
    Log::trace(">>> read sequence", 1);

    if (this->isCgiReadPipe(fd))
        this->receiveDataFromCgi(fd);
    else if (this->isStaticResource(fd))
        this->readStaticResource(fd);
    else if (this->isClientSocket(fd))
    {
        this->receiveRequest(fd);
        Log::getRequest(*this, fd);
        if (this->_requests[fd].getRecvRequest() == RecvRequest::COMPLETE)
        {
            this->_server_manager->fdClr(fd, FdSet::READ);
            this->processResponseBody(fd);
        }
    }

    Log::printTimeDiff(from, 1);
    Log::trace("<<< read sequence", 1);
}

void
Server::run(int fd)
{
    if (isServerSocket(fd))
        this->acceptClient();
    else
    {
        if (this->_server_manager->fdIsCopySet(fd, FdSet::WRITE))
        {
            try
            {
                this->writeSequence(fd);
            }
            catch(const SendErrorCodeToClientException& e)
            {
                std::cerr << e.what() << '\n';
            }
            catch(const CannotSendErrorCodeToClientException& e)
            {
                std::cerr << e.what() << '\n';
            }
            catch(const std::exception& e)
            {
                std::cerr << e.what() << '\n';
                this->findAndCloseClientSocket(fd);
            }
        }
        else if (this->_server_manager->fdIsCopySet(fd, FdSet::READ))
        {
            try
            {
                this->readSequence(fd);
            }
            catch(const SendErrorCodeToClientException& e)
            {
                if (this->isClientSocket(fd) && this->_requests[fd].getStatusCode().front() != '2')
                {
                    this->_server_manager->fdSet(fd, FdSet::WRITE);
                    this->_responses[fd].setStatusCode(this->_requests[fd].getStatusCode());
                }
                std::cerr << e.what() << '\n';
            }
            catch(const std::exception& e)
            {
                this->killCgiAndSendErrorToClient(fd);
                std::cerr << "[CODE 901] std::exception was throwed!: ";
                std::cerr << e.what() << '\n';
            }
        }
    }
}

void
Server::closeClientSocket(int client_fd)
{
    Response& response = this->_responses[client_fd];
    this->_server_manager->fdClr(client_fd, FdSet::READ);
    this->_server_manager->fdClr(client_fd, FdSet::WRITE);
    this->_server_manager->setClosedFdOnFdTable(client_fd);
    this->_server_manager->updateFdMax(client_fd);

    this->_requests[client_fd].init();
    if (response.isCgiWritePipeNotClosed())
        this->_server_manager->closeCgiWritePipe(*this, response.getWriteFdToCgi());
    if (response.isCgiReadPipeNotClosed())
        this->_server_manager->closeCgiReadPipe(*this, response.getReadFdFromCgi());
    if (this->_server_manager->isCgiProcessTerminated(client_fd) == false)
        this->_server_manager->terminateCgiProcess(client_fd);
    if (response.isResourceNotClosed())
        this->_server_manager->closeStaticResource(*this, response.getResourceFd());
    response.init();

    this->_server_manager->monitorTimeOutOff(client_fd);
    close(client_fd);
    Log::closeClient(*this, client_fd);
}

void
Server::closeFdAndSetClientOnWriteFdSet(int fd)
{
    const FdType& type = this->_server_manager->getFdTable()[fd].first; 
    int client_socket = this->_server_manager->getFdTable()[fd].second;

    this->_server_manager->fdClr(fd, FdSet::READ);
    this->_server_manager->setClosedFdOnFdTable(fd);
    this->_server_manager->updateFdMax(fd);
    this->_server_manager->fdSet(client_socket, FdSet::WRITE);
    close(fd);
    Log::closeFd(*this, client_socket, type, fd);
}

void
Server::closeFdAndUpdateFdTable(int fd, FdSet fd_set)
{
    this->_server_manager->fdClr(fd, fd_set);
    this->_server_manager->setClosedFdOnFdTable(fd);
    this->_server_manager->updateFdMax(fd);
    close(fd);
}

void
Server::closeFdAndSetFd(int clear_fd, FdSet clear_fd_set, int set_fd, FdSet set_fd_set)
{
    Log::trace("> closeFdAndSetFd", 2);
    timeval from;
    gettimeofday(&from, NULL);

    
    const FdType& type = this->_server_manager->getFdTable()[clear_fd].first;

    int client_socket = this->_server_manager->getFdTable()[clear_fd].second;
    Log::closeFd(*this, client_socket, type, clear_fd);

    this->_server_manager->fdClr(clear_fd, clear_fd_set);
    this->_server_manager->setClosedFdOnFdTable(clear_fd);
    this->_server_manager->updateFdMax(clear_fd);
    this->_server_manager->fdSet(set_fd, set_fd_set);
    close(clear_fd);

    gettimeofday(&from, NULL);
    Log::trace("< closeFdAndSetFd", 2);
}

bool
Server::isAuthRealm(int client_fd)
{
    const std::string& route = this->_responses[client_fd].getRoute();
    const location_info& location_info = this->_responses[client_fd].getLocationInfo();
    location_info::const_iterator it = location_info.find("auth_basic");
    if (it->second == "off")
        return (false);
    if (this->_authenticate_realms.find(route) == this->_authenticate_realms.end())
        return (false);
    return (true);
}

bool
Server::authorizationHeaderExists(int client_fd)
{
    if (this->_requests[client_fd].getHeaders().find("Authorization") ==
            this->_requests[client_fd].getHeaders().end())
        return (false);
    return (true);
}

void
Server::checkValidOfAuthHeader(int client_fd)
{
    std::string after_decode;
    Request& request = this->_requests[client_fd];
    Response& response = this->_responses[client_fd];
    std::vector<std::string> authenticate_info = ft::split(request.getHeaders().at("Authorization"), " ");

    if (authenticate_info.size() != 2)
        throw (AuthenticateErrorException(*this, client_fd, "403"));
    Base64::decode(authenticate_info[1], after_decode);
    for (auto& auth : this->_authenticate_realms.find(response.getRoute())->second)
    {
        if (auth == after_decode)
        {
            size_t pos = after_decode.find(":");
            request.setRemoteUser(after_decode.substr(0, pos));
            request.setRemoteIdent(after_decode.substr(pos + 1));
            return ;
        }
    }
    throw (AuthenticateErrorException(*this, client_fd, "403"));
}

void
Server::checkAuthenticate(int client_fd)
{
    std::string before_decode;
    std::string after_decode;

    if (this->isAuthRealm(client_fd) == false)
        return ;
    if (this->authorizationHeaderExists(client_fd) == false)
        throw (AuthenticateErrorException(*this, client_fd, "401"));
    this->checkValidOfAuthHeader(client_fd);
}

void
Server::parseUriAndSetResponse(int client_fd)
{
    Log::trace("> parseUriAndSetResponse", 2);
    timeval from;
    gettimeofday(&from, NULL);

    
    UriParser parser;
    if (parser.parseUri(this->_requests[client_fd].getUri()) == false)
        throw (Request::RequestFormatException(this->_requests[client_fd], "400"));
    const std::string& path = parser.getPath();

    Response& response = this->_responses[client_fd];
    response.setUriPath(path);
    response.setRouteAndLocationInfo(path, this);
    response.setQuery(parser.getQuery());
    std::string root = response.getLocationInfo().at("root");
    std::string file_path = path.substr(response.getRoute().length());
    response.setResourceAbsPath(root + file_path);

    
    Log::printTimeDiff(from, 2);
    Log::trace("< parseUriAndSetResponse", 2);
}

void 
Server::readStaticResource(int resource_fd)
{
    Log::trace("> readStaticResource", 1);
    timeval from;
    gettimeofday(&from, NULL);

    
    char buf[BUFFER_SIZE + 1];
    int bytes;
    int client_socket = this->_server_manager->getFdTable()[resource_fd].second;

    if ((bytes = read(resource_fd, buf, BUFFER_SIZE)) > 0)
    {
        buf[bytes] = 0;
        this->_responses[client_socket].appendBody(buf, bytes);
        if (bytes < BUFFER_SIZE)
            this->finishReadStaticResource(resource_fd);
        else
        {
            this->_responses[client_socket].setReceiveProgress(ReceiveProgress::ON_GOING);
            this->_server_manager->fdClr(resource_fd, FdSet::READ);
            this->_server_manager->fdSet(client_socket, FdSet::WRITE);
        }
    }
    else if (bytes == 0)
        this->finishReadStaticResource(resource_fd);
    else
        throw (ReadStaticResourceErrorException(*this, resource_fd));

    
    Log::printTimeDiff(from, 1);
    Log::trace("< readStaticResource", 1);
}

void
Server::openStaticResource(int client_fd)
{
    Log::trace("> openStaticResource", 1);
    timeval from;
    gettimeofday(&from, NULL);

    
    int resource_fd;
    const std::string& path = this->_responses[client_fd].getResourceAbsPath();
    struct stat tmp;

    if (this->_requests[client_fd].getMethod() == "PUT")
    {
        resource_fd = open(path.c_str(), O_CREAT | O_RDWR | O_TRUNC, 0666);
        if (resource_fd < 0)
            throw (InternalServerException(*this, client_fd));
        // resource_fd = open("/Users/sanam/Desktop/Webserv/abcd", O_CREAT | O_RDWR, 0666);
        // 여기서 파일이 오픈되지 않는다는 건 상위 폴더 자체가 없다는 것.
        // 409 에러를 보내줄 수 있게 만들자
        fcntl(resource_fd, F_SETFL, O_NONBLOCK);
        this->_server_manager->fdSet(resource_fd, FdSet::WRITE);
        this->_server_manager->setResourceOnFdTable(resource_fd, client_fd);
        this->_responses[client_fd].setResourceFd(resource_fd);
        this->_server_manager->updateFdMax(resource_fd);
        if ((fstat(resource_fd, &tmp)) == -1)
            throw OpenResourceErrorException(*this, client_fd, errno);
        this->_responses[client_fd].setFileInfo(tmp);
    }
    else if ((resource_fd = open(path.c_str(), O_RDWR, 0644)) > 0)
    {
        fcntl(resource_fd, F_SETFL, O_NONBLOCK);
        this->_server_manager->fdSet(resource_fd, FdSet::READ);
        this->_server_manager->setResourceOnFdTable(resource_fd, client_fd);
        this->_responses[client_fd].setResourceFd(resource_fd);
        this->_server_manager->updateFdMax(resource_fd);
        if ((fstat(resource_fd, &tmp)) == -1)
            throw OpenResourceErrorException(*this, client_fd, errno);
        this->_responses[client_fd].setFileInfo(tmp);
        Log::openFd(*this, client_fd, FdType::RESOURCE, resource_fd);
    }
    else
        throw OpenResourceErrorException(*this, client_fd, errno);

        
    Log::printTimeDiff(from, 1);
    Log::trace("< openStaticResource", 1);
}

void
Server::checkValidOfCgiMethod(int client_fd)
{
    const Request& request = this->_requests[client_fd];
    const std::string& method = request.getMethod();
    if (method != "GET" && method != "POST" && method != "HEAD")
        throw (CgiMethodErrorException(*this, client_fd));
}

void
Server::checkAndSetResourceType(int client_fd)
{
    Log::trace("> checkAndSetResourceType", 2);
    timeval from;
    gettimeofday(&from, NULL);


    Response& response = this->_responses[client_fd];
    const std::string& method = this->_requests[client_fd].getMethod();
    response.findAndSetUriExtension();
    if (this->isCgiUri(client_fd, response.getUriExtension()))
    {
        this->checkValidOfCgiMethod(client_fd);
        response.setResourceType(ResType::CGI);
        response.setCgiEnvpValues();
        return ;
    }
    DIR* dir_ptr;
    if ((dir_ptr = opendir(response.getResourceAbsPath().c_str())) != NULL)
    {
        if (response.getUriPath().back() != '/')
            response.setUriPath(response.getUriPath() + "/");
        response.setDirectoryEntry(dir_ptr);
        closedir(dir_ptr);
        if (method.compare("PUT") == 0)
            throw (CannotPutOnDirectoryException(response));
        if (this->isIndexFileExist(client_fd))
        {
            this->setResourceAbsPathAsIndex(client_fd);
            response.findAndSetUriExtension();
        }
        else
        {
            if (this->isAutoIndexOn(client_fd))
                response.setResourceType(ResType::AUTO_INDEX);
            else
                throw (IndexNoExistException(*this, client_fd));
        }
    }
    else
    {
        if (errno == ENOTDIR)
        {
            if (method == "PUT")
                response.setStatusCode("204");
            response.setResourceType(ResType::STATIC_RESOURCE);
        }
        else if (errno == EACCES)
            throw (CannotOpenDirectoryException(*this, client_fd, "403", errno));
        else if (errno == ENOENT)
        {
            if (method == "PUT")
            {
                response.setResourceType(ResType::STATIC_RESOURCE);
                response.setStatusCode("201");
                return ;
            }
            throw (CannotOpenDirectoryException(*this, client_fd, "404", errno));
        }
    }
    Request& request = this->_requests[client_fd];

    if (request.acceptLanguageHeaderExists() && this->_server_manager->isPluginOn("accept_language"))
        response.negotiateContent(request.getHeaders().at("Accept-Language"));

    Log::printTimeDiff(from, 2);
    Log::trace("< checkAndSetResourceType", 2);
}

void
Server::executePythonCgi(int client_fd)
{
    Py_Initialize();
    if (Py_IsInitialized())
    {
        char** argv = this->makeCgiArgv(client_fd);
        PySys_SetArgv(0, argv);
        PyRun_SimpleString("import sys\n");

        const std::string& python_script_uri = this->_responses[client_fd].getScriptName();
        const location_info& location_info = this->_responses[client_fd].getLocationInfo();

        const std::string& root_path = location_info.at("root");
        std::string python_script_path = root_path.substr(root_path.rfind("/") + 1);
        std::string python_script_name = python_script_uri.substr(location_info.at("root").length());
        python_script_name = python_script_name.substr(0, python_script_name.rfind(".py"));
        python_script_name[python_script_name.find("/")] = '.';
        python_script_path.append(python_script_name);

        std::string cgi_response;
        PyObject* py_module_object = PyImport_ImportModule(python_script_path.c_str());
        if (py_module_object)
        {
            PyObject* py_def_object = PyObject_GetAttrString(py_module_object, "python_cgi");
            if (py_def_object)
            {
                PyObject* result = PyObject_CallFunction(py_def_object, (char *)"s", this->_requests[client_fd].getBody().c_str());
                if (result)
                {
                    cgi_response += PyString_AS_STRING(result);
                    Py_XDECREF(result);
                }
                Py_XDECREF(py_def_object);
            }
            Py_XDECREF(py_module_object);
        }
        Py_Finalize();

        this->_responses[client_fd].setTempBuffer(cgi_response);
        this->_responses[client_fd].preparseCgiMessage();
        this->_responses[client_fd].setReceiveProgress(ReceiveProgress::FINISH);
        this->_server_manager->fdSet(client_fd, FdSet::WRITE);
    }
}

void
Server::preprocessResponseBody(int client_fd, ResType& res_type)
{
    Log::trace("> preprocessResponseBody", 1);
    timeval from;
    gettimeofday(&from, NULL);

    switch (res_type)
    {
    case ResType::AUTO_INDEX:
        this->_server_manager->fdSet(client_fd, FdSet::WRITE);
        break ;
    case ResType::STATIC_RESOURCE:
        this->openStaticResource(client_fd);
        break ;
    case ResType::CGI:
        if (this->_responses[client_fd].getUriExtension() == ".py")
            this->executePythonCgi(client_fd);
        else
        {
            this->openCgiPipe(client_fd);
            this->forkAndExecuteCgi(client_fd);
            if (this->_responses[client_fd].getUriExtension() == ".php")
                this->_responses[client_fd].setReceiveProgress(ReceiveProgress::PHP_CGI_BEGIN);
            else
                this->_responses[client_fd].setReceiveProgress(ReceiveProgress::CGI_BEGIN);
        }
        break ;
    default:
        this->_server_manager->fdSet(client_fd, FdSet::WRITE);
        break ;
    }
    
    Log::printTimeDiff(from, 1);
    Log::trace("< preprocessResponseBody", 1);
}

void
Server::deleteResourceOfUri(int client_fd, const std::string& path)
{
    Log::trace("> deleteResourceOfUri", 2);
    timeval from;
    gettimeofday(&from, NULL);

    Response& response = this->_responses[client_fd];
    DIR* dir_ptr;
    if ((dir_ptr = opendir(path.c_str())) != NULL)
    {
        if (path.back() != '/')
            throw (CannotOpenDirectoryException(*this, client_fd, "409", errno));
        std::vector<std::string> directory_entry = ft::makeDirectoryEntry(dir_ptr);
        closedir(dir_ptr);
        for (const std::string& entry : directory_entry)
        {
            if (entry != "./" && entry != "../")
                this->deleteResourceOfUri(client_fd, path + entry);
        }
        if (rmdir(path.c_str()) == -1)
            throw (InternalServerException(*this, client_fd));
    }
    else
    {
        if (errno == ENOTDIR)
        {
            if (unlink(path.c_str()) == -1)
                throw (TargetResourceConflictException(*this, client_fd));
        }
        else
            throw (CannotOpenDirectoryException(*this, client_fd, "404", errno));
    }
    response.setStatusCode("204");
    Log::printTimeDiff(from, 2);
    Log::trace("< deleteResourceOfUri", 2);
}

void
Server::processResponseBody(int client_fd)
{
    Log::trace("> processResopnseBody", 1);
    timeval from;
    gettimeofday(&from, NULL);

    const location_info& location_info = this->_responses[client_fd].getLocationInfo();
    if (location_info.find("limit_client_body_size") != location_info.end())
    {
        if (this->_requests[client_fd].getBody().length() > static_cast<unsigned int>(std::stoi(location_info.at("limit_client_body_size"))))
            throw (PayloadTooLargeException(*this, client_fd));
    }

    if (this->_responses[client_fd].isLimitExceptInLocation() && 
        !(this->_responses[client_fd].isAllowedMethod(this->_requests[client_fd].getMethod())))
            throw (NotAllowedMethodException(*this, client_fd));
    
    if (this->_responses[client_fd].isLocationToBeRedirected())
        throw (MustRedirectException(*this, client_fd));
    if (this->_requests[client_fd].getMethod().compare("DELETE") == 0)
    {
        this->deleteResourceOfUri(client_fd, this->_responses[client_fd].getResourceAbsPath());
        this->_server_manager->fdSet(client_fd, FdSet::WRITE);
        return ;
    }
    this->checkAndSetResourceType(client_fd);
    if (this->_requests[client_fd].getMethod().compare("OPTIONS") == 0)
    {
        this->_server_manager->fdSet(client_fd, FdSet::WRITE);
        return ;
    }
    ResType res_type = this->_responses[client_fd].getResourceType();

    this->preprocessResponseBody(client_fd, res_type);

    Log::printTimeDiff(from, 1);
    Log::trace("< processResopnseBody", 1);
}

void
Server::openCgiPipe(int client_fd)
{
    Log::trace("> openCgiPipe", 1);
    timeval from;
    gettimeofday(&from, NULL);
    
    
    Response& response = this->_responses[client_fd];
    int pipe1[2];
    int pipe2[2];

    if (pipe(pipe1) < 0)
        throw (InternalServerException(*this, client_fd));
    if (pipe(pipe2) < 0)
    {
        close(pipe1[0]);
        close(pipe1[1]);
        throw (InternalServerException(*this, client_fd));
    }

    int stdin_of_cgi = pipe1[0];
    int stdout_of_cgi = pipe2[1];
    int read_fd_from_cgi = pipe2[0];
    int write_fd_to_cgi = pipe1[1];
    Log::openFd(*this, client_fd, FdType::PIPE, read_fd_from_cgi);
    Log::openFd(*this, client_fd, FdType::PIPE, write_fd_to_cgi);

    response.setStdinOfCgi(stdin_of_cgi);
    response.setStdoutOfCgi(stdout_of_cgi);
    response.setReadFdFromCgi(read_fd_from_cgi);
    response.setWriteFdToCgi(write_fd_to_cgi);

    fcntl(stdin_of_cgi, F_SETFL, O_NONBLOCK);
    fcntl(stdout_of_cgi, F_SETFL, O_NONBLOCK);
    fcntl(read_fd_from_cgi, F_SETFL, O_NONBLOCK);
    fcntl(write_fd_to_cgi, F_SETFL, O_NONBLOCK);

    this->_server_manager->setCgiPipeOnFdTable(read_fd_from_cgi, client_fd);
    this->_server_manager->setCgiPipeOnFdTable(write_fd_to_cgi, client_fd);
    this->_server_manager->updateFdMax(read_fd_from_cgi);
    this->_server_manager->updateFdMax(write_fd_to_cgi);

    
    Log::printTimeDiff(from, 1);
    Log::trace("< openCgiPipe", 1);
}

bool
Server::makeEnvpUsingRequest(char** envp, int client_fd, int* idx)
{
    Request& request= this->_requests[client_fd];
    if (!(envp[(*idx)++] = ft::strdup("AUTH_TYPE=" + request.getAuthType())))
        return (false);
    if (!(envp[(*idx)++] = ft::strdup("REMOTE_USER=" + request.getRemoteUser())))
        return (false);
    if (!(envp[(*idx)++] = ft::strdup("REMOTE_IDENT=" + request.getRemoteIdent())))
        return (false);
    if (!(envp[(*idx)++] = ft::strdup("REMOTE_ADDR=" + request.getIpAddress())))
        return (false);
    if (!(envp[(*idx)++] = ft::strdup("REQUEST_METHOD="+ request.getMethod())))
        return (false);
    return (true);
}

bool
Server::makeEnvpUsingResponse(char** envp, int client_fd, int* idx)
{
    const Response& response = this->_responses[client_fd];
    
    std::string path_info;
    std::string path_translated;
    if (response.getPathInfo().length() == 0)
    {
        path_info = response.getUriPath();
        path_translated = response.getResourceAbsPath();
    }
    else
    {
        path_info = response.getPathInfo();
        path_translated = response.getPathTranslated();
    }
    if (!(envp[(*idx)++] = ft::strdup("PATH_INFO=" + path_info)))
        return (false);
    if (!(envp[(*idx)++] = ft::strdup("PATH_TRANSLATED=" + path_translated)))
        return (false);
    if (!(envp[(*idx)++] = ft::strdup("REQUEST_URI=" + response.getRequestUriForCgi())))
        return (false);
    if (!(envp[(*idx)++] = ft::strdup("SCRIPT_NAME=" + response.getScriptName())))
        return (false);
    if (!(envp[(*idx)++] = ft::strdup("QUERY_STRING=" + response.getQuery())))
        return (false);
    return (true);
}

bool
Server::makeEnvpUsingHeaders(char** envp, int client_fd, int* idx)
{
    const std::map<std::string, std::string>& headers = this->_requests[client_fd].getHeaders();
    std::map<std::string, std::string>::const_iterator it;

    it = headers.find("Content-Length");
    if (it == headers.end())
    {
        if (!(envp[(*idx)++] = ft::strdup("CONTENT_LENGTH=")))
            return (false);
    }
    else
    {
        if (!(envp[(*idx)++] = ft::strdup("CONTENT_LENGTH=" + it->second)))
            return (false);
    }
    it = headers.find("Content-Type");
    if (it == headers.end())
    {
        if (!(envp[(*idx)++] = ft::strdup("CONTENT_TYPE=text/html")))
            return (false);
    }
    else
    {
        if (!(envp[(*idx)++] = ft::strdup("CONTENT_TYPE=" + it->second)))
            return (false);
    }
    for (auto& kv : headers)
    {
        if (!(envp[(*idx)++] = ft::strdup("HTTP_" + kv.first + "=" + kv.second)))
            return (false);
    }
    return (true);
}

bool
Server::makeEnvpUsingEtc(char** envp, int* idx)
{
    if (!(envp[(*idx)++] = ft::strdup("GATEWAY_INTERFACE=Cgi/1.1")))
        return (false);
    if (!(envp[(*idx)++] = ft::strdup("SERVER_NAME=" + this->getHost())))
        return (false);
    if (!(envp[(*idx)++] = ft::strdup("SERVER_PORT=" + this->getPort())))
        return (false);
    if (!(envp[(*idx)++] = ft::strdup("SERVER_PROTOCOL=HTTP/1.1")))
        return (false);
    if (!(envp[(*idx)++] = ft::strdup("SERVER_SOFTWARE=GET_POLAR_BEAR/2.0")))
        return (false);
    return (true);
}

char**
Server::makeCgiEnvp(int client_fd)
{
    Log::trace("> makeCgiEnvp", 2);
    timeval from;
    gettimeofday(&from, NULL);
    
    
    int idx = 0;
    char** envp;
    int num_of_envp;

    num_of_envp = NUM_OF_META_VARIABLES + this->_requests[client_fd].getHeaders().size();
    if (!(envp = (char **)malloc(sizeof(char *) * num_of_envp)))
        throw (InternalServerException(*this, client_fd));
    for (int i = 0; i < num_of_envp; i++)
        envp[i] = nullptr;
    if (!this->makeEnvpUsingRequest(envp, client_fd, &idx) ||
        !this->makeEnvpUsingResponse(envp, client_fd, &idx) ||
        !this->makeEnvpUsingHeaders(envp, client_fd, &idx) ||
        !this->makeEnvpUsingEtc(envp, &idx))
    {
        ft::doubleFree(&envp);
        throw (InternalServerException(*this, client_fd));
    }

    Log::printTimeDiff(from, 2);
    Log::trace("< makeCgiEnvp", 2);
    return (envp);
}

char**
Server::makeCgiArgv(int client_fd)
{
    Log::trace("> makeCgiArgv", 2);
    timeval from;
    gettimeofday(&from, NULL);

    char** argv;

    if (!(argv = (char **)malloc(sizeof(char *) * 2)))
        throw (InternalServerException(*this, client_fd));
    for (int i = 0; i < 2; i++)
        argv[i] = nullptr;
    if (!(argv[0] = ft::strdup(this->_responses[client_fd].getScriptName())))
    {
        ft::doubleFree(&argv);
        throw (InternalServerException(*this, client_fd));
    }


    Log::printTimeDiff(from, 2);
    Log::trace("< makeCgiArgv", 2);
    return (argv);
}

bool
Server::isResponseAllSended(int fd) const
{
    return (this->_responses[fd].getParseProgress() == ParseProgress::FINISH &&
            this->_responses[fd].getSendProgress() == SendProgress::ALL_SENDED);
}

void
Server::forkAndExecuteCgi(int client_fd)
{
    Log::trace("> forkAndExecuteCgi", 1);
    timeval from;
    gettimeofday(&from, NULL);

    pid_t pid;
    int ret;
    int stdin_of_cgi;
    int stdout_of_cgi;
    char** argv;
    char** envp;
    Response& response = this->_responses[client_fd];

    stdin_of_cgi = response.getStdinOfCgi();
    stdout_of_cgi = response.getStdoutOfCgi();
    if (!(argv = this->makeCgiArgv(client_fd)))
    {
        ft::doubleFree(&argv);
        throw (InternalServerException(*this, client_fd));
    }
    if (!(envp = this->makeCgiEnvp(client_fd)))
    {
        ft::doubleFree(&argv);
        ft::doubleFree(&envp);
        throw (InternalServerException(*this, client_fd));
    }

    if ((pid = fork()) < 0)
        throw (InternalServerException(*this, client_fd));
    else if (pid == 0)
    {
        close(response.getWriteFdToCgi());
        close(response.getReadFdFromCgi());
        if (dup2(stdin_of_cgi, 0) < 0)
            throw (InternalServerException(*this, client_fd));
        if (dup2(stdout_of_cgi, 1) < 0)
            throw (InternalServerException(*this, client_fd));
        
        if (this->_responses[client_fd].getUriExtension() == ".php" &&
            (ret = execve(PHP_CGI_PATH.c_str(), argv, envp)) < 0)
            exit(ret);
        if ((ret = execve(argv[0], argv, envp)) < 0)
            exit(ret);
        exit(ret);
    }
    else
    {
        close(stdin_of_cgi);
        close(stdout_of_cgi);
        g_child_process_ids[client_fd] = pid;
        response.setCgiPid(pid);
        ft::doubleFree(&argv);
        ft::doubleFree(&envp);
        this->_server_manager->fdSet(response.getWriteFdToCgi(), FdSet::WRITE);
    }

    Log::printTimeDiff(from, 1);
    Log::trace("< forkAndExecuteCgi", 1);
}

void
Server::finishSendDataToCgiPipe(int write_fd_to_cgi)
{
    this->_server_manager->closeCgiWritePipe(*this, write_fd_to_cgi);

    int client_fd = this->_server_manager->getLinkedFdFromFdTable(write_fd_to_cgi);
    this->_server_manager->fdSet(this->_responses[client_fd].getReadFdFromCgi(), FdSet::READ);
}

void
Server::finishReceiveDataFromCgiPipe(int read_fd_from_cgi)
{
    int status;
    int client_fd = this->_server_manager->getLinkedFdFromFdTable(read_fd_from_cgi);
    Response& response = this->_responses[client_fd];

    g_child_process_ids[client_fd] = DEFAULT_PID;
    waitpid(response.getCgiPid(), &status, 0);

    this->_server_manager->closeCgiReadPipe(*this, read_fd_from_cgi);

    this->_server_manager->fdSet(client_fd, FdSet::WRITE);
    if (response.getSendProgress() == SendProgress::READY)
        throw (InternalServerException(*this, client_fd));
    this->_server_manager->fdSet(client_fd, FdSet::WRITE);
    response.setReadFdFromCgi(DEFAULT_FD);
    response.setReceiveProgress(ReceiveProgress::FINISH);
}

void
Server::finishReadStaticResource(int resource_fd)
{
    int client_fd = this->_server_manager->getLinkedFdFromFdTable(resource_fd);

    this->_server_manager->closeStaticResource(*this, resource_fd);

    this->_server_manager->fdSet(client_fd, FdSet::WRITE);
    this->_responses[client_fd].setReceiveProgress(ReceiveProgress::FINISH);
}
void
Server::finishPutFileOnServer(int resource_fd)
{
    int client_fd = this->_server_manager->getLinkedFdFromFdTable(resource_fd);

    this->_server_manager->closeStaticResource(*this, resource_fd);
    this->_server_manager->fdSet(client_fd, FdSet::WRITE);
}

void
Server::killCgiAndSendErrorToClient(int fd)
{
    int client_fd;
    if (this->isCgiReadPipe(fd))
    {
        client_fd = this->_server_manager->getLinkedFdFromFdTable(fd);
        kill(this->_responses[client_fd].getCgiPid(), SIGTERM);
        g_child_process_ids[client_fd] = DEFAULT_PID;
    }
    else if (this->isStaticResource(fd))
        client_fd = this->_server_manager->getLinkedFdFromFdTable(fd);
    else if (this->isClientSocket(fd))
        client_fd = fd;
    else
        client_fd = this->_server_manager->getLinkedFdFromFdTable(fd);
    this->_server_manager->fdSet(client_fd, FdSet::WRITE);
    this->_responses[client_fd].setStatusCode("500");
}

void
Server::findAndCloseClientSocket(int fd)
{
    int client_fd;
    if (this->isCgiWritePipe(fd))
        client_fd = this->_server_manager->getLinkedFdFromFdTable(fd);
    else if (this->isClientSocket(fd))
        client_fd = fd;
    else
        client_fd = this->_server_manager->getLinkedFdFromFdTable(fd);
    this->closeClientSocket(client_fd);
}
