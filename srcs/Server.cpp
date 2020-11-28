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

/*============================================================================*/
/******************************  Constructor  *********************************/
/*============================================================================*/

//TODO: _limit_client_body_size -> ServerGenerator에서 만들어 주도록 변경할 것.
Server::Server(ServerManager* server_manager, server_info& server_config, std::map<std::string, location_info>& location_config)
: _server_manager(server_manager), _server_config(server_config),
_server_socket(-1), _server_name(""), _host(server_config["server_name"]), _port(""),
_request_uri_limit_size(0), _request_header_limit_size(0), 
_limit_client_body_size(150000000), _location_config(location_config)
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
Server::setAuthenticateRealm()
{
    int fd;
    char temp[1024];
    std::string before_decode;
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
                    ft::memset(reinterpret_cast<void *>(temp), 0, 1024);
                    int bytes = read(fd, temp, 1024);
                    if (bytes >= 0)
                    {
                        before_decode = std::string(temp);
                        Base64::decode(before_decode, after_decode);
                        this->setAuthBasicUserFile(after_decode, location.first);
                    }
                    else
                        this->setAuthBasic("off", location.first);
                }
            }
        }
    }
}


/*============================================================================*/
/******************************  Exception  ***********************************/
/*============================================================================*/

Server::PayloadTooLargeException::PayloadTooLargeException(Response& response) 
: _response(response)
{
    this->_response.setStatusCode("413");
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
    this->_response.setStatusCode("404");
}

const char*
Server::IndexNoExistException::what() const throw()
{
    return ("[CODE 404] No index & Autoindex off");
}

Server::CgiMethodErrorException::CgiMethodErrorException(Response& response)
: _response(response)
{
    this->_response.setStatusCode("400");
}

const char*
Server::CgiMethodErrorException::what() const throw()
{
    return ("[CODE 400] CGI can handle only GET HEAD POST");
}

Server::InternalServerException::InternalServerException(Response& response)
: _response(response)
{
    this->_response.setStatusCode("500");
}

const char*
Server::InternalServerException::what() const throw()
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

Server::TargetResourceConflictException::TargetResourceConflictException(Response& response)
: _response(response)
{
    this->_response.setStatusCode("409");
}

const char*
Server::TargetResourceConflictException::what() const throw()
{
    return ("[CODE 409] Target resource conflict exception");
}

const char*
Server::UnchunkedErrorException::what() const throw()
{
    return ("[CODE 902] Chunked request couldn't receive or Receive error");
}

Server::NotAllowedMethodException::NotAllowedMethodException(Response& response)
: _response(response)
{
    this->_response.setStatusCode("405");
}

const char*
Server::NotAllowedMethodException::what() const throw()
{
    return ("[CODE 405] Not Allowed Method");
}

const char*
Server::CannotWriteToClientException::what() const throw()
{
    return ("[CODE 901] Cannot write to client");
}

/*============================================================================*/
/*********************************  Util  *************************************/ 
/*============================================================================*/

void
Server::init()
{
    for (auto& conf: this->_server_config)
    {
        if (conf.first == "host")
            this->_host = conf.second;
        else if (conf.first == "listen")
            this->_port = conf.second;
    }
    this->_requests = std::vector<Request>(1024);
    this->_responses = std::vector<Response>(1024);
    this->setAuthenticateRealm();

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
Server::isExistCRLFInChunkSize(int fd)
{
    return (this->_requests[fd].getIndexOfCRLFInChunkSize() != -1);
}

bool
Server::isNotYetSetTargetChunkSize(int fd)
{
    return (this->_requests[fd].getTargetChunkSize() == DEFAULT_TARGET_CHUNK_SIZE);
}

int
Server::readBufferUntilRequestLine(int client_fd, char* buf, size_t line_end_pos)
{
    Log::trace("> readBufferUntilRequestLine", 1);
    timeval from;
    gettimeofday(&from, NULL);

    int bytes;
    Request& request = this->_requests[client_fd];

    if ((bytes = read(client_fd, buf, line_end_pos + CRLF_SIZE)) > 0)
        buf[bytes] = 0;
    else if (bytes == 0)
        throw (Request::RequestFormatException(request, "400"));
    else
        throw (ReadErrorException());
    return (bytes);

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
        request.appendTempBuffer(buf, bytes); // bytes 가 peeked보다 작을 수 있다.
        return (static_cast<size_t>(bytes) == read_target_size);
    }
    else if (bytes == 0)
        throw (Request::RequestFormatException(request, "400"));
    else
        throw (ReadErrorException());
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
                throw (ReadErrorException());
        }
        else if (bytes == 0)
            throw (ReadErrorException());
        else
            throw (ReadErrorException());
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
    size_t line_end_pos = 0;
    Request& request = this->_requests[client_fd];

    if ((bytes = request.peekMessageFromClient(client_fd, buf)) > 0)
    {
        buf[bytes] = 0;
        std::string readed(buf, bytes);
        if ((line_end_pos = readed.find("\r\n")) != std::string::npos)
        {
            if (line_end_pos >= BUFFER_SIZE - 2)
                throw (Request::UriTooLongException(request));
            bytes = this->readBufferUntilRequestLine(client_fd, buf, line_end_pos);
            request.parseRequestLine(buf, bytes);
            request.setRecvRequest(RecvRequest::HEADERS);
        }
        else
            throw (Request::RequestFormatException(request, "400"));
    }
    else if (bytes == RECV_COUNT_NOT_REACHED)
        request.raiseRecvCounts();
    else if (bytes == 0)
        this->closeClientSocket(client_fd);
    else
        throw (ReadErrorException());

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
                if (peeked_bytes == read_target_size) // header까지 다읽었는데 버퍼에 남은게 없다.
                {
                    if (request.isBodyUnnecessary() ||
                        (request.isNormalBody() && request.getHeaders().at("Content-Length") == "0"))
                        request.setRecvRequest(RecvRequest::COMPLETE);
                }
                request.updateRecvRequest();
            }
        }
    }
    else if (peeked_bytes == RECV_COUNT_NOT_REACHED)
        request.raiseRecvCounts();
    else if (peeked_bytes == 0)
        this->closeClientSocket(client_fd);
    else
        throw (ReadErrorException());

    
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
    if (content_length > this->_limit_client_body_size)
        throw (PayloadTooLargeException(this->_responses[client_fd]));

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
            throw (PayloadTooLargeException(this->_responses[client_fd]));
        if (bytes < BUFFER_SIZE)
            request.setRecvRequest(RecvRequest::COMPLETE);
    }
    else if (bytes == 0)
        this->closeClientSocket(client_fd);
    else
        throw (ReadErrorException());

    Log::printTimeDiff(from, 1);
    Log::trace("< receiveRequestNormalBody", 1);
}

void
Server::receiveChunkSize(int client_fd, size_t index_of_crlf)
{
    Log::trace("> receiveChunkSize", 1);
    timeval from;
    gettimeofday(&from, NULL);

    int bytes;
    char buf[RECEIVE_SOCKET_STREAM_SIZE + 1];
    Request& request = this->_requests[client_fd];

    int received_chunk_size_length = request.getReceivedChunkSizeLength();

    // if ((bytes = recv(client_fd, buf, index_of_crlf + CRLF_SIZE, 0)) > 0)
    if ((bytes = recv(client_fd, buf, 1, 0)) > 0)
    {
        buf[bytes] = 0;
        received_chunk_size_length += bytes;
        request.setReceivedChunkSizeLength(received_chunk_size_length);

        if (received_chunk_size_length == static_cast<int>(index_of_crlf) + CRLF_SIZE)
        {
            request.appendChunkSize(buf, bytes); //NOTE: CRLF가 포함된 버퍼가 stoiHex로 들어가도 문제없음 확인
            request.parseTargetChunkSize(request.getChunkSize());
            request.setIndexOfCRLFInChunkSize(-1);
            request.setChunkSize("");
            request.setReceivedChunkSizeLength(0);
        }
        else
            request.appendChunkSize(buf, bytes);
    }
    else if (bytes == 0)
        this->closeClientSocket(client_fd);
    else
        throw (UnchunkedErrorException());

    Log::printTimeDiff(from, 1);
    Log::trace("< receiveChunkSize", 1);
}

// bool flag;

void
Server::receiveChunkData(int client_fd, int receive_size)
{

    Log::trace("> receiveChunkData", 1);
    timeval from;
    gettimeofday(&from, NULL);

    int bytes;
    char buf[RECEIVE_SOCKET_STREAM_SIZE + 1];
    Request& request = this->_requests[client_fd];

    // ft::memset(buf, 0, RECEIVE_SOCKET_STREAM_SIZE + 1);
    if ((bytes = recv(client_fd, buf, receive_size, 0)) > 0)
    {
        buf[bytes] = 0;
        // readed_bytes += bytes;
        // std::cout<<"\033[1;33m"<<"in receiveChunkData bytes: "<<bytes<<"\033[0m"<<std::endl;
        // std::cout<<"\033[1;33m"<<"in receiveChunkData readed_bytes: "<<readed_bytes<<"\033[0m"<<std::endl;
        request.setReceivedChunkDataLength(request.getReceivedChunkDataLength() + bytes);
        request.parseChunkData(buf, bytes);
        // request.setRemainedChunkDataSize(receive_size - bytes);
        // if (flag == true)
        // {
        //     std::cout << "\033[33m\033[01m";
        // std::cout << "===============================================" << std::endl;
        //     std::cout << "bytes: " << bytes << std::endl;
        //     std::cout << "receive_target size: " << receive_size << std::endl;
        //     std::cout << "target_chunk_size: " << request.getTargetChunkSize() << std::endl;
        //     std::cout << "received_chunk_data_size: " << request.getReceivedChunkDataLength() << std::endl;
        // std::cout << "===============================================" << std::endl;
        // std::cout << "\033[0m";
        //     flag = false;
        // }

        // std::cout << "\033[35m\033[01m";
        // if (bytes != receive_size)
        // {
        // std::cout << "===============================================" << std::endl;
        //     std::cout << "bytes: " << bytes << std::endl;
        //     std::cout << "receive size: " << receive_size << std::endl;
        //     std::cout << "target_chunk_size: " << request.getTargetChunkSize() << std::endl;
        //     std::cout << "received_chunk_data_size: " << request.getReceivedChunkDataLength() << std::endl;
        // std::cout << "request body len: " << request.getBody().length() << std::endl;
        // std::cout << "next target: " << receive_size - bytes << std::endl;
        // std::cout << "===============================================" << std::endl;
        // flag = true;
        // // sleep(100);
        // }
        // std::cout << "\033[0m";
    }
    else if (bytes == 0)
    {
        // std::cout<<"\033[1;31m"<<"received Bytes: 0"<<"\033[0m"<<std::endl;
        this->closeClientSocket(client_fd);
    }
    else
        throw (UnchunkedErrorException());

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

    // bytes = recv(client_fd, buf, CRLF_SIZE + 1, 0);
    bytes = recv(client_fd, buf, 1, 0);
    if (bytes > 0)
    {
        buf[bytes] = 0;
        received_last_chunk_data_length += bytes;
        request.setReceivedLastChunkDataLength(received_last_chunk_data_length);

        if (received_last_chunk_data_length < CRLF_SIZE)
            request.appendLastChunkData(buf, bytes);
        else if (received_last_chunk_data_length > CRLF_SIZE)
            throw (Request::RequestFormatException(request, "400"));
        else
        {
            if ((is_buffer_left = recv(client_fd, buf, 1, MSG_PEEK)) > 0)
                throw (Request::RequestFormatException(request, "400"));
            request.appendLastChunkData(buf, bytes);
            if (request.getLastChunkData().compare("\r\n") == 0)
            {
                request.setRecvRequest(RecvRequest::COMPLETE);
                request.setLastChunkData("");
                request.setReceivedLastChunkDataLength(0);
                request.setTargetChunkSize(DEFAULT_TARGET_CHUNK_SIZE);
            }
            else
                throw (Request::RequestFormatException(request, "400"));
        }
    }
    else if (bytes == 0)
        this->closeClientSocket(client_fd);
    else
        throw (UnchunkedErrorException());

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
    size_t index_of_crlf;

    if ((bytes = request.peekMessageFromClient(client_fd, buf)) > 0)
    {
        buf[bytes] = 0;
        if (this->isExistCRLFInChunkSize(client_fd))
            this->receiveChunkSize(client_fd, request.getIndexOfCRLFInChunkSize());
        else if (this->isNotYetSetTargetChunkSize(client_fd))
        {
            if ((index_of_crlf = std::string(buf).find("\r\n")) != std::string::npos)
                this->_requests[client_fd].setIndexOfCRLFInChunkSize(index_of_crlf);
            else
                throw (Request::RequestFormatException(request, "400"));
        }
        else if (request.getTargetChunkSize() == 0)
            this->receiveLastChunkData(client_fd);
        else
        {
            int receive_target_size = std::min(RECEIVE_SOCKET_STREAM_SIZE, request.getTargetChunkSize() + CRLF_SIZE - request.getReceivedChunkDataLength());
            this->receiveChunkData(client_fd, receive_target_size);

            if (request.getTargetChunkSize() + CRLF_SIZE == request.getReceivedChunkDataLength())
            {
                request.setReceivedChunkDataLength(0);
                request.setTargetChunkSize(DEFAULT_TARGET_CHUNK_SIZE);
            }
        }
    }
    else if (bytes == RECV_COUNT_NOT_REACHED)
        request.raiseRecvCounts();
    else if (bytes == 0)
        this->closeClientSocket(client_fd);
    else
        throw (UnchunkedErrorException());
        
        
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
        // this->processResponseBody()
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
Server::makeResponseMessage(int client_fd)
{
    Log::trace("> makeResponseMessage", 1);
    timeval from;
    gettimeofday(&from, NULL);

    Request& request = this->_requests[client_fd];
    Response& response = this->_responses[client_fd];
    const std::string& method = request.getMethod();

    std::string status_line;
    std::string headers;

    if (response.getStatusCode().front() != '2')
        response.setResourceType(ResType::ERROR_HTML);

    if (response.isRedirection(response.getStatusCode()) == false)
        response.makeBody(request);

    const SendProgress& send_progress = response.getSendProgress();
    if (send_progress == SendProgress::READY)
    {
        headers = response.makeHeaders(request);
        status_line = response.makeStatusLine();
    }

    //TODO: refactoring
    const ParseProgress& parse_progress = response.getParseProgress();
    switch (parse_progress)
    {
    case ParseProgress::FINISH:
        if (method == "HEAD")
            response.setResponseMessage("");

        Log::printTimeDiff(from, 1);
        Log::trace("< makeResponseMessage", 1);
        response.setResponseMessage(response.getTransmittingBody());
        break;
    case ParseProgress::DEFAULT:
        response.setParseProgress(ParseProgress::FINISH);
        if (method == "HEAD" || ((method == "PUT" || method == "DELETE") && response.getStatusCode().front() == '2'))
            response.setResponseMessage(status_line + headers);

        Log::printTimeDiff(from, 1);
        Log::trace("< makeResponseMessage", 1);
        response.setResponseMessage(status_line + headers + response.getTransmittingBody());
        break;
    case ParseProgress::CHUNK_START:
        if (request.getMethod() == "HEAD")
            response.setResponseMessage(status_line + headers);
            
        Log::printTimeDiff(from, 1);
        Log::trace("< makeResponseMessage", 1);
        response.setResponseMessage(status_line + headers + response.getTransmittingBody());
        break;
    case ParseProgress::CHUNK_PROGRESS:
        if (request.getMethod() == "HEAD")
            response.setResponseMessage("");

        Log::printTimeDiff(from, 1);
        Log::trace("< makeResponseMessage", 1);
        response.setResponseMessage(response.getTransmittingBody());
        break;
    default:
        Log::printTimeDiff(from, 1);
        Log::trace("< makeResponseMessage", 1);
        break;
    }
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
        // std::cout<<response_message<<std::endl;
        // std::cout<<"\033[1;44;37m"<<"sended_bytes: "<<sended_bytes<<"\033[0m"<<std::endl;
        // std::cout<<"\033[1;44;37m"<<"ParseProgress: "<<Log::parseProgressToString(this->_responses[client_fd].getParseProgress())<<"\033[0m"<<std::endl;

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
        throw (CannotWriteToClientException());
    else
        throw (CannotWriteToClientException());

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
Server::isCGIPipe(int fd) const
{
    Log::trace("> isCGIPipe", 2);
    timeval from;
    gettimeofday(&from, NULL);


    const std::vector<std::pair<FdType, int> >& fd_table = this->_server_manager->getFdTable();
    if (fd_table[fd].first == FdType::PIPE)
    {
        Log::printTimeDiff(from, 2);
        Log::trace("< isCGIPipe return true", 2);
        return true;
    }


    Log::printTimeDiff(from, 2);
    Log::trace("< isCGIPipe return false", 2);
    return false;
}

bool
Server::isCGIWritePipe(int fd) const
{
    Log::trace("> isCGIWritePipe", 2);
    timeval from;
    gettimeofday(&from, NULL);


    const std::vector<std::pair<FdType, int> >& fd_table = this->_server_manager->getFdTable();
    if (fd_table[fd].first == FdType::PIPE && this->_responses[fd_table[fd].second].getWriteFdToCGI() == fd)
    {
        Log::printTimeDiff(from, 2);
        Log::trace("< isCGIWritePipe return true", 2);
        return true;
    }
    Log::printTimeDiff(from, 2);
    Log::trace("< isCGIWritePipe return false", 2);
    return false;
}

bool
Server::isCGIReadPipe(int fd) const
{
    Log::trace("> isCGIReadPipe", 2);
    timeval from;
    gettimeofday(&from, NULL);

    
    const std::vector<std::pair<FdType, int> >& fd_table = this->_server_manager->getFdTable();
    if (fd_table[fd].first == FdType::PIPE && this->_responses[fd_table[fd].second].getReadFdFromCGI() == fd)
    {
        Log::printTimeDiff(from, 2);
        Log::trace("< isCGIReadPipe return true", 2);
        return true;
    }


    Log::printTimeDiff(from, 2);
    Log::trace("< isCGIReadPipe return false", 2);
    return false;
}

bool
Server::isIndexFileExist(int client_fd)
{
    const std::string& dir_entry = this->_responses[client_fd].getDirectoryEntry();
    const location_info& location_info = this->_responses[client_fd].getLocationInfo();
    std::vector<std::string> indexs = ft::split(location_info.at("index"), " ");
    for (std::string& index : indexs)
    {
        if (dir_entry.find(index) != std::string::npos)
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
Server::isCGIUri(int client_fd, const std::string& extension)
{
    Log::trace("> isCGIUri", 2);
    timeval from;
    gettimeofday(&from, NULL);

    if (extension == "")
    {
        Log::printTimeDiff(from, 2);
        Log::trace("< isCGIUri return false", 2);
        return (false);
    }

    const location_info& location_info = this->_responses[client_fd].getLocationInfo();
    location_info::const_iterator it = location_info.find("cgi");
    if (it == location_info.end())
    {
        Log::printTimeDiff(from, 2);
        Log::trace("< isCGIUri return false", 2);
        return (false);
    }

    const std::string& cgi = it->second;
    if (cgi.find(extension) == std::string::npos)
    {
        Log::printTimeDiff(from, 2);
        Log::trace("< isCGIUri return false", 2);
        return (false);
    }

    Log::printTimeDiff(from, 2);
    Log::trace("< isCGIUri return true", 2);
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
Server::sendDataToCGI(int write_fd_to_cgi)
{
    Log::trace("> sendDataToCGI", 1);
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
            this->closeFdAndSetFd(write_fd_to_cgi, FdSet::WRITE, response.getReadFdFromCGI(), FdSet::READ);
        else
        {
            this->_server_manager->fdSet(response.getReadFdFromCGI(), FdSet::READ);
            this->_server_manager->fdClr(write_fd_to_cgi, FdSet::WRITE);
            this->_server_manager->fdClr(client_fd, FdSet::READ);
        }
    }
    else if (bytes == 0)
        this->closeFdAndSetFd(write_fd_to_cgi, FdSet::WRITE, response.getReadFdFromCGI(), FdSet::READ);
    else
        throw (InternalServerException(this->_responses[client_fd]));

    Log::printTimeDiff(from, 1);
    Log::trace("< sendDataToCGI", 1);
}

void
Server::receiveDataFromCGI(int read_fd_from_cgi)
{
    Log::trace("> receiveDataFromCGI", 1);
    timeval from;
    gettimeofday(&from, NULL);


    int bytes;
    int client_fd;
    int status;

    client_fd = this->_server_manager->getLinkedFdFromFdTable(read_fd_from_cgi);
    Response& response = this->_responses[client_fd];

    char buf[BUFFER_SIZE + 1];
    // ft::memset(static_cast<void *>(buf), 0, BUFFER_SIZE + 1);
    //TODO: usleep 필요 여부와 적정 수치 조정 필요함.
    // usleep(1000);
    bytes = read(read_fd_from_cgi, buf, BUFFER_SIZE);
    if (bytes > 0)
    {
        buf[bytes] = 0;
        response.appendBody(buf, bytes);
        if (response.getReceiveProgress() == ReceiveProgress::CGI_BEGIN)
            response.preparseCGIMessage();
        this->_server_manager->fdClr(read_fd_from_cgi, FdSet::READ);
        this->_server_manager->fdSet(client_fd, FdSet::WRITE);
        response.setReceiveProgress(ReceiveProgress::ON_GOING);
    }
    else if (bytes == 0)
    {
        waitpid(response.getCGIPid(), &status, 0);
        this->closeFdAndSetFd(read_fd_from_cgi, FdSet::READ, client_fd, FdSet::WRITE);
        response.setReceiveProgress(ReceiveProgress::FINISH);
    }
    else
        throw (InternalServerException(this->_responses[client_fd]));


    Log::printTimeDiff(from, 1);
    Log::trace("< receiveDataFromCGI", 1);
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
            if (bytes == remained)
                this->closeFdAndSetFd(resource_fd, FdSet::WRITE, client_fd, FdSet::WRITE);
            transfered_body_size += bytes;
            request.setTransferedBodySize(transfered_body_size);
        }
        else if (bytes == 0)
        {
            if (remained > 0)
                throw (InternalServerException(this->_responses[resource_fd]));
            this->closeFdAndSetFd(resource_fd, FdSet::WRITE, client_fd, FdSet::WRITE);
        }
        else
            throw (InternalServerException(this->_responses[resource_fd]));
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
                throw (InternalServerException(this->_responses[resource_fd]));
            this->closeFdAndSetFd(resource_fd, FdSet::WRITE, client_fd, FdSet::WRITE);
        }
        else
            throw (InternalServerException(this->_responses[resource_fd]));
    }
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
                timeval from;
                gettimeofday(&from, NULL);
                Log::trace(">>> write sequence", 1);
                if (this->isCGIWritePipe(fd))
                    this->sendDataToCGI(fd);
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
                            if (this->_server_manager->getFdType(this->_responses[fd].getWriteFdToCGI()) != FdType::CLOSED)
                                this->_server_manager->fdSet(this->_responses[fd].getWriteFdToCGI(), FdSet::WRITE);
                            else
                                this->_server_manager->fdSet(this->_responses[fd].getReadFdFromCGI(), FdSet::READ);
                        }
                    }
                    if (this->isResponseAllSended(fd))
                    {
                        if (this->_responses[fd].getStatusCode()[0] != '2')
                        {
                            this->_server_manager->fdClr(fd, FdSet::WRITE);
                            this->_responses[fd].init();
                            this->closeClientSocket(fd);
                        }
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
            catch(const SendErrorCodeToClientException& e)
            {
                if (this->isCGIWritePipe(fd))
                {
                    int client_fd = this->_server_manager->getLinkedFdFromFdTable(fd);
                    this->closeFdAndSetFd(this->_responses[client_fd].getWriteFdToCGI(), FdSet::WRITE, client_fd, FdSet::WRITE);
                    this->closeFdAndUpdateFdTable(this->_responses[client_fd].getReadFdFromCGI(), FdSet::READ);
                }
                else if (this->isClientSocket(fd))
                {
                }
                else
                {
                    int client_fd = this->_server_manager->getLinkedFdFromFdTable(fd);
                    //TODO 
                    this->closeFdAndSetFd(fd, FdSet::WRITE, client_fd, FdSet::WRITE);
                    this->closeFdAndUpdateFdTable(fd, FdSet::READ);
                }
                std::cerr << e.what() << '\n';
            }
            catch(const std::exception& e)
            {
                if (this->_responses[fd].getResourceFd() != DEFAULT_FD)
                    this->closeFdAndUpdateFdTable(this->_responses[fd].getResourceFd(), FdSet::READ);
                else
                {
                    if (this->_server_manager->getFdType(this->_responses[fd].getWriteFdToCGI()) != FdType::CLOSED)
                        this->closeFdAndUpdateFdTable(this->_responses[fd].getWriteFdToCGI(), FdSet::READ);
                    if (this->_server_manager->getFdType(this->_responses[fd].getReadFdFromCGI()) != FdType::CLOSED)
                        this->closeFdAndUpdateFdTable(this->_responses[fd].getReadFdFromCGI(), FdSet::READ);
                }
                this->closeClientSocket(fd);
                std::cerr << e.what() << '\n';
            }
            catch(const char* e)
            {
                std::cerr << e << '\n';
            }
        }
        else if (this->_server_manager->fdIsCopySet(fd, FdSet::READ))
        {
            try
            {
                if (this->isCGIReadPipe(fd))
                    this->receiveDataFromCGI(fd);
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
            }
            catch(const SendErrorCodeToClientException& e)
            {
                std::cerr << e.what() << '\n';
                this->_server_manager->fdSet(fd, FdSet::WRITE);
                if (this->_responses[fd].getWriteFdToCGI() != DEFAULT_FD ||
                        this->_responses[fd].getReadFdFromCGI() != DEFAULT_FD)
                {
                    Response& response = this->_responses[fd];
                    this->closeFdAndUpdateFdTable(response.getReadFdFromCGI(), FdSet::READ);
                    this->closeFdAndUpdateFdTable(response.getWriteFdToCGI(), FdSet::WRITE);
                }
                //TODO: static resource 추가
                // else if (this->_)
            }
            catch(const Request::RequestFormatException& e)
            {
                std::cerr << e.what() << '\n';
                this->_server_manager->fdSet(fd, FdSet::WRITE);
                this->_responses[fd].setStatusCode(this->_requests[fd].getStatusCode());
            }
            catch(const Request::UriTooLongException& e)
            {
                std::cerr << e.what() << '\n';
                this->_server_manager->fdSet(fd, FdSet::WRITE);
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
Server::closeClientSocket(int client_fd)
{
    this->_server_manager->fdClr(client_fd, FdSet::READ);
    this->_server_manager->setClosedFdOnFdTable(client_fd);
    this->_server_manager->updateFdMax(client_fd);
    this->_requests[client_fd].init();
    this->_server_manager->monitorTimeOutOff(client_fd);
    close(client_fd);
    Log::closeClient(*this, client_fd);
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
    Log::closeFd(*this, set_fd, type, clear_fd);

    this->_server_manager->fdClr(clear_fd, clear_fd_set);
    this->_server_manager->setClosedFdOnFdTable(clear_fd);
    this->_server_manager->updateFdMax(clear_fd);
    this->_server_manager->fdSet(set_fd, set_fd_set);
    close(clear_fd);


    gettimeofday(&from, NULL);
    Log::trace("< closeFdAndSetFd", 2);
}


void
Server::checkAuthenticate(int client_fd)
{
    std::string before_decode;
    std::string after_decode;
    Response& response = this->_responses[client_fd];
    Request& request = this->_requests[client_fd];
    const std::string& route = response.getRoute();
    const std::map<std::string, location_info>& location_config = this->getLocationConfig();
    const location_info& location_info = location_config.at(route);
    location_info::const_iterator it = location_info.find("auth_basic");
    if (it->second == "off")
        return ;
    it = location_info.find("auth_basic_user_file");
    if (it == location_info.end())
        return ;
    const std::string& id_password = it->second;
    const std::map<std::string, std::string>& headers = this->_requests[client_fd].getHeaders();
    it = headers.find("Authorization");
    if (it == headers.end())
        throw (AuthenticateErrorException(this->_responses[client_fd], "401"));
    std::vector<std::string> authenticate_info = ft::split(it->second, " ");
    if (authenticate_info[0] != "Basic")
        throw (AuthenticateErrorException(this->_responses[client_fd], "401"));
    before_decode = authenticate_info[1];
    Base64::decode(before_decode, after_decode);
    if (id_password != after_decode)
        throw (AuthenticateErrorException(this->_responses[client_fd], "403"));
    request.setAuthType(authenticate_info[0]);
    size_t pos = after_decode.find(":");
    request.setRemoteUser(after_decode.substr(0, pos));
    request.setRemoteIdent(after_decode.substr(pos + 1));
}

//TODO: 함수명이 기능을 담지 못함, 수정 필요함!
//NOTE: 1. 
void
Server::findResourceAbsPath(int client_fd)
{
    Log::trace("> findResourceAbsPath", 2);
    timeval from;
    gettimeofday(&from, NULL);

    
    UriParser parser;
    parser.parseUri(this->_requests[client_fd].getUri());
    const std::string& path = parser.getPath();

    Response& response = this->_responses[client_fd];
    response.setUriPath(path);
    response.setRouteAndLocationInfo(path, this);
    std::string root = response.getLocationInfo().at("root");
    // if (response.getRoute() != "/")
    //     root.pop_back();
    std::string file_path = path.substr(response.getRoute().length());
    response.setResourceAbsPath(root + file_path);
    // /Users/iwoo/Documents/Webserv/YoupiBanane/youpi.bad_extension

    
    Log::printTimeDiff(from, 2);
    Log::trace("< findResourceAbsPath", 2);
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

    // ft::memset(buf, 0, BUFFER_SIZE + 1);
    if ((bytes = read(resource_fd, buf, BUFFER_SIZE)) > 0)
    {
        buf[bytes] = 0;
        this->_responses[client_socket].appendBody(buf, bytes);
        if (bytes < BUFFER_SIZE)
        {
            this->_responses[client_socket].setReceiveProgress(ReceiveProgress::FINISH);
            this->closeFdAndSetFd(resource_fd, FdSet::READ, client_socket, FdSet::WRITE);
        }
        else
        {
            this->_responses[client_socket].setReceiveProgress(ReceiveProgress::ON_GOING);
            this->_server_manager->fdClr(resource_fd, FdSet::READ);
            this->_server_manager->fdSet(client_socket, FdSet::WRITE);
        }
    }
    else if (bytes == 0)
        this->closeFdAndSetFd(resource_fd, FdSet::READ, client_socket, FdSet::WRITE);
    else
    {
        this->closeFdAndSetFd(resource_fd, FdSet::READ, client_socket, FdSet::WRITE);
        throw (InternalServerException(this->_responses[client_socket]));
    }

    
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
            throw InternalServerException(this->_responses[client_fd]);
        // resource_fd = open("/Users/sanam/Desktop/Webserv/abcd", O_CREAT | O_RDWR, 0666);
        // 여기서 파일이 오픈되지 않는다는 건 상위 폴더 자체가 없다는 것.
        // 409 에러를 보내줄 수 있게 만들자
        fcntl(resource_fd, F_SETFL, O_NONBLOCK);
        this->_server_manager->fdSet(resource_fd, FdSet::WRITE);
        this->_server_manager->setResourceOnFdTable(resource_fd, client_fd);
        this->_responses[client_fd].setResourceFd(resource_fd);
        this->_server_manager->updateFdMax(resource_fd);
        if ((fstat(resource_fd, &tmp)) == -1)
            throw OpenResourceErrorException(this->_responses[client_fd], errno);
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
            throw OpenResourceErrorException(this->_responses[client_fd], errno);
        this->_responses[client_fd].setFileInfo(tmp);
        Log::openFd(*this, client_fd, FdType::RESOURCE, resource_fd);
    }
    else
        throw OpenResourceErrorException(this->_responses[client_fd], errno);

        
    Log::printTimeDiff(from, 1);
    Log::trace("< openStaticResource", 1);
}

void
Server::checkValidOfCgiMethod(int client_fd)
{
    const Request& request = this->_requests[client_fd];
    const std::string& method = request.getMethod();
    if (method != "GET" && method != "POST" && method != "HEAD")
        throw (CgiMethodErrorException(this->_responses[client_fd]));
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
    if (this->isCGIUri(client_fd, response.getUriExtension()))
    {
        this->checkValidOfCgiMethod(client_fd);
        response.setResourceType(ResType::CGI);
        return ;
    }
    DIR* dir_ptr;
    if ((dir_ptr = opendir(response.getResourceAbsPath().c_str())) != NULL)
    {
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
                throw (IndexNoExistException(this->_responses[client_fd]));
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
            throw (CannotOpenDirectoryException(this->_responses[client_fd], "403", errno));
        else if (errno == ENOENT)
        {
            if (method == "PUT")
            {
                response.setResourceType(ResType::STATIC_RESOURCE);
                response.setStatusCode("201");
                return ;
            }
            throw (CannotOpenDirectoryException(this->_responses[client_fd], "404", errno));
        }
    }

    Log::printTimeDiff(from, 2);
    Log::trace("< checkAndSetResourceType", 2);
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
        this->openCGIPipe(client_fd);
        this->forkAndExecuteCGI(client_fd);
        this->_responses[client_fd].setReceiveProgress(ReceiveProgress::CGI_BEGIN);
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
            throw (CannotOpenDirectoryException(response, "409", errno));
        response.setDirectoryEntry(dir_ptr);
        closedir(dir_ptr);
        std::vector<std::string> directory_entry = ft::split(response.getDirectoryEntry(), " ");
        for (std::string& entry : directory_entry)
        {
            if (entry != "./" && entry != "../")
                this->deleteResourceOfUri(client_fd, path + entry);
        }
        if (rmdir(path.c_str()) == -1)
            throw (InternalServerException(response));
    }
    else
    {
        if (errno == ENOTDIR) // dicrectory가 아니다ㅏ. -> unlink
        {
            if (unlink(path.c_str()) == -1)
                throw (TargetResourceConflictException(response));
        }
        else
            throw (CannotOpenDirectoryException(response, "404", errno));
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

    //NOTE: 수정 필요함
    this->findResourceAbsPath(client_fd);

    const location_info& location_info = this->_responses[client_fd].getLocationInfo();
    if (location_info.find("limit_client_body_size") != location_info.end())
    {
        if (this->_requests[client_fd].getBody().length() > static_cast<unsigned int>(std::stoi(location_info.at("limit_client_body_size"))))
            throw (PayloadTooLargeException(this->_responses[client_fd]));
    }

    this->checkAuthenticate(client_fd);
    if (this->_responses[client_fd].isLimitExceptInLocation() && 
        !(this->_responses[client_fd].isAllowedMethod(this->_requests[client_fd].getMethod())))
            throw (NotAllowedMethodException(this->_responses[client_fd]));
    
    if (this->_responses[client_fd].isLocationToBeRedirected())
        throw (MustRedirectException(this->_responses[client_fd]));
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
Server::openCGIPipe(int client_fd)
{
    Log::trace("> openCGIPipe", 1);
    timeval from;
    gettimeofday(&from, NULL);
    
    
    Response& response = this->_responses[client_fd];
    int pipe1[2];
    int pipe2[2];

    if (pipe(pipe1) < 0)
        throw (InternalServerException(this->_responses[client_fd]));
    if (pipe(pipe2) < 0)
        throw (InternalServerException(this->_responses[client_fd]));

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

    
    Log::printTimeDiff(from, 1);
    Log::trace("< openCGIPipe", 1);
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
    
    if (!(envp[(*idx)++] = ft::strdup("PATH_INFO=" + response.getUriPath())))
        return (false);
    if (!(envp[(*idx)++] = ft::strdup("PATH_TRANSLATED=" + response.getResourceAbsPath())))
        return (false);
    if (!(envp[(*idx)++] = ft::strdup("REQUEST_URI=" + response.getUriPath())))
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
Server::makeEnvpUsingEtc(char** envp, int client_fd, int* idx)
{
    const std::map<std::string, std::string>& location_info =
        this->getLocationConfig().at(this->_responses[client_fd].getRoute());

    if (!(envp[(*idx)++] = ft::strdup("GATEWAY_INTERFACE=CGI/1.1")))
        return (false);
    if (!(envp[(*idx)++] = ft::strdup("SCRIPT_NAME=" + location_info.at("cgi_path"))))
        return (false);
    if (!(envp[(*idx)++] = ft::strdup("SERVER_NAME=" + this->getHost())))
        return (false);
    if (!(envp[(*idx)++] = ft::strdup("SERVER_PORT=" + this->getPort())))
        return (false);
    if (!(envp[(*idx)++] = ft::strdup("SERVER_PROTOCOL=HTTP/1.1")))
        return (false);
    if (!(envp[(*idx)++] = ft::strdup("SERVER_SOFTWARE=GET_POLAR_BEAR/2.0")))
        return (false);
    if (!(envp[(*idx)++] = ft::strdup("QUERY_STRING=")))
        return (false);
    return (true);
}

char**
Server::makeCGIEnvp(int client_fd)
{
    Log::trace("> makeCGIEnvp", 2);
    timeval from;
    gettimeofday(&from, NULL);
    
    
    int idx = 0;
    char** envp;
    int num_of_envp;

    num_of_envp = NUM_OF_META_VARIABLES + this->_requests[client_fd].getHeaders().size();
    if (!(envp = (char **)malloc(sizeof(char *) * num_of_envp)))
        throw (InternalServerException(this->_responses[client_fd]));
    for (int i = 0; i < num_of_envp; i++)
        envp[i] = nullptr;
    if (!this->makeEnvpUsingRequest(envp, client_fd, &idx) ||
        !this->makeEnvpUsingResponse(envp, client_fd, &idx) ||
        !this->makeEnvpUsingHeaders(envp, client_fd, &idx) ||
        !this->makeEnvpUsingEtc(envp, client_fd, &idx))
    {
        ft::doubleFree(&envp);
        throw (InternalServerException(this->_responses[client_fd]));
    }


    Log::printTimeDiff(from, 2);
    Log::trace("< makeCGIEnvp", 2);
    return (envp);
}

char**
Server::makeCGIArgv(int client_fd)
{
    Log::trace("> makeCGIArgv", 2);
    timeval from;
    gettimeofday(&from, NULL);


    char** argv;
    Response& response = this->_responses[client_fd];

    if (!(argv = (char **)malloc(sizeof(char *) * 3)))
        throw (InternalServerException(this->_responses[client_fd]));
    const location_info& location_info =
            this->getLocationConfig().at(this->_responses[client_fd].getRoute());
    for (int i = 0; i < 3; i++)
        argv[i] = nullptr;
    location_info::const_iterator it = location_info.find("cgi_path");
    if (it == location_info.end())
    {
        ft::doubleFree(&argv);
        throw (InternalServerException(this->_responses[client_fd]));
    }
    if (!(argv[0] = ft::strdup(location_info.at("cgi_path"))))
    {
        ft::doubleFree(&argv);
        throw (InternalServerException(this->_responses[client_fd]));
    }
    if (!(argv[1] = ft::strdup(response.getResourceAbsPath())))
    {
        ft::doubleFree(&argv);
        throw (InternalServerException(this->_responses[client_fd]));
    }

    
    Log::printTimeDiff(from, 2);
    Log::trace("< makeCGIArgv", 2);
    return (argv);
}

bool
Server::isResponseAllSended(int fd) const
{
    return (this->_responses[fd].getParseProgress() == ParseProgress::FINISH &&
            this->_responses[fd].getSendProgress() == SendProgress::ALL_SENDED);
}

void
Server::forkAndExecuteCGI(int client_fd)
{
    Log::trace("> forkAndExecuteCGI", 1);
    timeval from;
    gettimeofday(&from, NULL);

    pid_t pid;
    int ret;
    int stdin_of_cgi;
    int stdout_of_cgi;
    char** argv;
    char** envp;
    Response& response = this->_responses[client_fd];

    stdin_of_cgi = response.getStdinOfCGI();
    stdout_of_cgi = response.getStdoutOfCGI();
    if (!(argv = this->makeCGIArgv(client_fd)))
    {
        ft::doubleFree(&argv);
        throw (InternalServerException(this->_responses[client_fd]));
    }
    if (!(envp = this->makeCGIEnvp(client_fd)))
    {
        ft::doubleFree(&argv);
        ft::doubleFree(&envp);
        throw (InternalServerException(this->_responses[client_fd]));
    }
    if ((pid = fork()) < 0)
        throw (InternalServerException(this->_responses[client_fd]));
    else if (pid == 0)
    {
        close(response.getWriteFdToCGI());
        close(response.getReadFdFromCGI());
        if (dup2(stdin_of_cgi, 0) < 0)
            throw (InternalServerException(this->_responses[client_fd]));
        if (dup2(stdout_of_cgi, 1) < 0)
            throw (InternalServerException(this->_responses[client_fd]));
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
        this->_server_manager->fdSet(response.getWriteFdToCGI(), FdSet::WRITE);
    }
    Log::printTimeDiff(from, 1);
    Log::trace("< forkAndExecuteCGI", 1);
}
