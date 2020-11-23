#include "Request.hpp"
#include "Log.hpp"

/*============================================================================*/
/****************************  Static variables  ******************************/
/*============================================================================*/

/*============================================================================*/
/******************************  Constructor  *********************************/
/*============================================================================*/

//TODO: target_chunk_size 상수화 하기
Request::Request()
: _method(""), _uri(""), _version(""),
_protocol(""), _body(""), _status_code("200"),
_info(ReqInfo::READY), _is_buffer_left(false),
_ip_address(""), _transfered_body_size(0), _target_chunk_size(DEFAULT_TARGET_CHUNK_SIZE),
_received_chunk_data_size(0), _recv_counts(0), _carriege_return_trimmed(false)
 {}

Request::Request(const Request& other)
: _method(other._method), _uri(other._uri), 
_version(other._version), _headers(other._headers),
_protocol(other._protocol), _body(other._body),
_status_code(other._status_code), _info(other._info),
_is_buffer_left(other._is_buffer_left), _ip_address(other._ip_address),
_transfered_body_size(other._transfered_body_size), _target_chunk_size(other._target_chunk_size),
_received_chunk_data_size(other._received_chunk_data_size), _recv_counts(other._recv_counts),
_carriege_return_trimmed(other._carriege_return_trimmed)
{}

Request&
Request::operator=(const Request& other)
{
    this->_method = other._method;
    this->_uri = other._uri;
    this->_version = other._version;
    this->_headers = other._headers;
    this->_protocol = other._protocol;
    this->_body = other._body;
    this->_status_code = other._status_code;
    this->_info = other._info;
    this->_is_buffer_left = other._is_buffer_left;
    this->_ip_address = other._ip_address;
    this->_transfered_body_size = other._transfered_body_size;
    this->_target_chunk_size = other._target_chunk_size;
    this->_received_chunk_data_size = other._received_chunk_data_size;
    this->_recv_counts = other._recv_counts;
    this->_carriege_return_trimmed = other._carriege_return_trimmed;
    return (*this);
}

/*============================================================================*/
/******************************  Destructor  **********************************/
/*============================================================================*/

Request::~Request() {}

/*============================================================================*/
/********************************  Getter  ************************************/
/*============================================================================*/

const std::string&
Request::getMethod() const
{
    return (this->_method);
}

const std::string&
Request::getUri() const
{
    return (this->_uri);
}

const std::string&
Request::getVersion() const
{
    return (this->_version);
}

const std::map<std::string, std::string>&
Request::getHeaders() const
{
    return (this->_headers);
}

const std::string&
Request::getProtocol() const
{
    return (this->_protocol);
}

const std::string&
Request::getBody() const
{
    return (this->_body);
}

const std::string&
Request::getStatusCode() const
{
    return (this->_status_code);
}

const ReqInfo&
Request::getReqInfo() const
{
    return (this->_info);
}

bool
Request::getIsBufferLeft() const
{
    return (this->_is_buffer_left);
}

const std::string&
Request::getIpAddress() const
{
    return (this->_ip_address);
}

int
Request::getTransferedBodySize() const
{
    return (this->_transfered_body_size);
}

const std::string&
Request::getAuthType() const
{
    return (this->_auth_type);
}

const std::string&
Request::getRemoteUser() const
{
    return (this->_remote_user);
}

const std::string&
Request::getRemoteIdent() const
{
    return (this->_remote_ident);
}

int
Request::getTargetChunkSize() const
{
    return (this->_target_chunk_size);
}

int
Request::getReceivedChunkDataSize() const
{
    return (this->_received_chunk_data_size);
}

int
Request::getReceiveCounts() const
{
    return (this->_recv_counts);
}

bool
Request::getCarriegeReturnTrimmed() const
{
    return (this->_carriege_return_trimmed);
}

/*============================================================================*/
/********************************  Setter  ************************************/
/*============================================================================*/

void
Request::setMethod(const std::string& method)
{
    this->_method = method;
}

void
Request::setUri(const std::string& uri)
{
    this->_uri = uri;
}

void
Request::setVersion(const std::string& version)
{
    this->_version = version;
}

void
Request::setHeaders(const std::string& key, const std::string& value)
{
    this->_headers[key] = value;
}

void
Request::setProtocol(const std::string& protocol)
{
    this->_protocol = protocol;
}

void
Request::setBody(const std::string& req_message)
{
    this->_body = req_message;
}

void
Request::setStatusCode(const std::string& status_code)
{
    this->_status_code = status_code;
}

void
Request::setReqInfo(const ReqInfo& info)
{
    this->_info = info;
}

void
Request::setIsBufferLeft(const bool& is_left_buffer)
{
    this->_is_buffer_left = is_left_buffer;
}

void
Request::setIpAddress(const std::string& ip_address)
{
    this->_ip_address = ip_address;
}

void
Request::setTransferedBodySize(const int transfered_body_size)
{
    this->_transfered_body_size = transfered_body_size;
}

void
Request::setAuthType(const std::string& auth_type)
{
    this->_auth_type = auth_type;
}

void
Request::setRemoteUser(const std::string& remote_user)
{
    this->_remote_user = remote_user;
}

void
Request::setRemoteIdent(const std::string& remote_ident)
{
    this->_remote_ident = remote_ident;
}

void
Request::setTargetChunkSize(const int target_size)
{
    this->_target_chunk_size = target_size;
}

void
Request::setReceivedChunkDataSize(const int received_chunk_data_size)
{
    this->_received_chunk_data_size = received_chunk_data_size;
}

void
Request::setReceiveCounts(const int recv_counts)
{
    this->_recv_counts = recv_counts;
}

void
Request::setCarriegeReturnTrimmed(const bool carriege_return)
{
    this->_carriege_return_trimmed = carriege_return;
}

/*============================================================================*/
/******************************  Exception  ***********************************/
/*============================================================================*/

Request::RequestFormatException::RequestFormatException(Request& req, const std::string& status_code)
: _msg("RequestFormatException: Invalid Request Format: " + status_code), _req(req) 
{
    this->_req.setStatusCode(status_code);
}

const char*
Request::RequestFormatException::what() const throw()
{
    return (this->_msg.c_str());
}

/*============================================================================*/
/*********************************  Util  *************************************/
/*============================================================================*/

std::ostream& operator<< (std::ostream& out, Request& object)
{
    out << "============================= result =============================\n";
    out << object.getMethod() << "\n";
    out << object.getUri() << "\n";
    out << object.getVersion() << "\n";

    for (auto& kv : object.getHeaders())
    {
        out << kv.first << ": ";
        out << kv.second << "\n";
    }
    out << "============================= end =============================" << "\n";
    return (out);
}

void
Request::updateReqInfo()
{
    Log::trace("> updateReqInfo", 2);
    timeval from;
    gettimeofday(&from, NULL);

    if (this->getReqInfo() == ReqInfo::COMPLETE)
        return ;
    if (this->getMethod() == "" && this->getUri() == "" && this->getVersion() == "")
        this->setReqInfo(ReqInfo::READY);
    else if (this->isBodyUnnecessary())
        this->setReqInfo(ReqInfo::MUST_CLEAR);
    else if (this->isNormalBody())
        this->setReqInfo(ReqInfo::NORMAL_BODY);
    else if (this->isChunkedBody())
        this->setReqInfo(ReqInfo::CHUNKED_BODY);

    Log::printTimeDiff(from, 2);
    Log::trace("< updateReqInfo", 2);
}

bool
Request::isBodyUnnecessary() const
{
    const std::string& method = this->getMethod();
    if (method.compare("PUT") == 0 || method.compare("POST") == 0)
        return (false);
    return (true);
}

bool
Request::isNormalBody() const
{
    if (this->getReqInfo() == ReqInfo::COMPLETE)
        return (false);

    const std::map<std::string, std::string>& headers = this->getHeaders();
    const location_info::const_iterator it = headers.find("Transfer-Encoding");
    if (it != headers.end() && (it->second.find("chunked") != std::string::npos))
        return (false);
    return (true);
}

bool
Request::isChunkedBody() const
{
    if (this->getReqInfo() == ReqInfo::COMPLETE)
        return (false);
    return (!isNormalBody());
}


bool
Request::isContentLeftInBuffer() const
{
    return (this->getIsBufferLeft());
}

int
Request::peekMessageFromClient(int client_fd, char* buf)
{
    int bytes = recv(client_fd, buf, BUFFER_SIZE, MSG_PEEK);
    //TODO 50은 임의값임. 최적값 찾아서 매크로상수화할 것.
    if (bytes <= 0 || bytes == BUFFER_SIZE || this->getReceiveCounts() == 50)
    {
        this->setReceiveCounts(0);
        return (bytes);
    }
    return (RECV_COUNT_NOT_REACHED);
}

void
Request::raiseRecvCounts()
{
    this->_recv_counts++;
}

bool
Request::updateStatusCodeAndReturn(const std::string& status_code, const bool& ret)
{
    this->setStatusCode(status_code);
    return (ret);
}

void
Request::parseRequestWithoutBody(char* buf, int bytes)
{
    Log::trace("> parseRequestWithoutBody", 1);
    timeval from;
    gettimeofday(&from, NULL);

    std::string line;
    std::string req_message(buf, bytes);

    if (ft::substr(line, req_message, "\r\n") == false)
        throw (RequestFormatException(*this, "400"));
    else
    {
        if (this->parseRequestLine(line) == false)
            throw (RequestFormatException(*this, "400"));
    }
    if (ft::substr(line, req_message, "\r\n\r\n") == false)
        throw (RequestFormatException(*this, "400"));
    else
    {
        if (this->parseHeaders(line) == false)
            throw (RequestFormatException(*this, "400"));
    }
    this->updateReqInfo();

    Log::printTimeDiff(from, 1);
    Log::trace("< parseRequestWithoutBody", 1);
}

bool
Request::parseRequestLine(std::string& req_message)
{
    Log::trace("> parseRequestLine", 2);
    timeval from;
    gettimeofday(&from, NULL);

    std::vector<std::string> request_line = ft::split(req_message, " ");
    
    if (this->isValidLine(request_line) == false)
        return (false);
    this->setMethod(request_line[0]);
    this->setUri(request_line[1]);
    this->setVersion(request_line[2]);

    Log::printTimeDiff(from, 2);
    Log::trace("< parseRequestLine", 2);
    return (true);
}

bool
Request::parseHeaders(std::string& req_message)
{
    Log::trace("> parseHeaders", 2);
    timeval from;
    gettimeofday(&from, NULL);

    std::string key;
    std::string value;
    std::string line;

    while (ft::substr(line, req_message, "\r\n") && !req_message.empty())
    {
        if (ft::substr(key, line, ":") == false)
            return (this->updateStatusCodeAndReturn("400", false));
        value = ft::ltrim(line, " ");
        if (this->isValidHeaders(key, value) == false)
            return (this->updateStatusCodeAndReturn("400", false));
        this->setHeaders(key, value);
    }
    if (ft::substr(key, line, ":") == false)
        return (this->updateStatusCodeAndReturn("400", false));
    value = ft::ltrim(line, " ");
    if (this->isValidHeaders(key, value) == false)
        return (this->updateStatusCodeAndReturn("400", false));
    this->setHeaders(key, value);


    Log::printTimeDiff(from, 2);
    Log::trace("< parseHeaders", 2);
    return (true);
}

// int received;

void
Request::parseTargetChunkSize(const std::string& chunk_size_line)
{
    int target_chunk_size;

    target_chunk_size = ft::stoiHex(chunk_size_line);
    
    // received += target_chunk_size;
    // std::cout<<"\033[1;30;43m"<<"received: "<<received<<"\033[0m"<<std::endl;
    if (target_chunk_size == -1)
    {
        // std::cout<<"\033[1;31m"<<"chunk_size_line: "<<chunk_size_line<<"\033[0m"<<std::endl;
        std::cout<<"\033[1;31m"<<"In parseTargetChunkSize throw~!"<<"\033[0m"<<std::endl;
        throw (RequestFormatException(*this, "400"));
    }
    this->setTargetChunkSize(target_chunk_size);
}

void
Request::parseChunkDataAndSetChunkSize(char* buf, size_t bytes, int next_target_chunk_size)
{
    if (bytes == RECEIVE_SOCKET_STREAM_SIZE)
        this->appendBody(buf, bytes);
    else
        this->appendBody(buf, bytes - 2);
    this->setTargetChunkSize(next_target_chunk_size);
}

void
Request::parseChunkData(char* buf, size_t bytes)
{

    if (bytes <= CRLF_SIZE)
        return ;
    if (this->isCarriegeReturnTrimmed())
    {
        this->appendBody("\r", 1);
        this->setCarriegeReturnTrimmed(false);
    }
    if (bytes == RECEIVE_SOCKET_STREAM_SIZE && buf[bytes - 2] == '\r' && buf[bytes - 1] == '\n')
        this->appendBody(buf, bytes - 2);
    else if (bytes == RECEIVE_SOCKET_STREAM_SIZE && buf[bytes - 1] == '\r')
    {
        this->setCarriegeReturnTrimmed(true);
        this->appendBody(buf, bytes - 1);
    }
    else if (bytes == RECEIVE_SOCKET_STREAM_SIZE)
        this->appendBody(buf, bytes);
    else
        this->appendBody(buf, bytes - CRLF_SIZE);
}

void
Request::appendBody(char* buf, int bytes)
{
    this->_body.append(buf, bytes);
}

void
Request::appendBody(const char* buf, int bytes)
{
    this->_body.append(buf, bytes);
}

void
Request::init()
{
    this->_method = "";
    this->_uri = "";
    this->_version = "";
    this->_headers = {};
    this->_protocol = "";
    this->_body = "";
    this->_status_code = "200";
    this->_info = ReqInfo::READY;
    this->_is_buffer_left = false;
    this->_ip_address = "";
    this->_transfered_body_size = 0;
    this->_target_chunk_size = DEFAULT_TARGET_CHUNK_SIZE;
    this->_received_chunk_data_size = 0;
    this->_recv_counts = 0;
}

int
Request::getContentLength() const
{
    location_info::const_iterator it = this->_headers.find("Content-Length");
    if (it == this->_headers.end())
        return (0);
    return (std::stoi(it->second));
}

/*============================================================================*/
/*****************************  Valid Check  **********************************/
/*============================================================================*/

bool
Request::isValidLine(std::vector<std::string>& request_line)
{
    if (request_line.size() != 3 ||
        this->isValidMethod(request_line[0]) == false ||
        this->isValidUri(request_line[1]) == false ||
        this->isValidVersion(request_line[2]) == false)
        return (false);
    return (true);
}

bool
Request::isValidMethod(const std::string& method)
{
    if (method.compare("GET") == 0 ||
        method.compare("POST") == 0 ||
        method.compare("PUT") == 0 ||
        method.compare("HEAD") == 0 ||
        method.compare("DELETE") == 0 ||
        method.compare("OPTIONS") == 0 ||
        method.compare("TRACE") == 0 ||
        method.compare("CONNECT") == 0)
        return (true);
    return (this->updateStatusCodeAndReturn("501", false));
}

//TODO: uri 유효성 검사 부분 더 알아보기.
bool
Request::isValidUri(const std::string& uri)
{
    if (uri[0] == '/' || uri[0] == 'w')
        return (true);
    return (this->updateStatusCodeAndReturn("400", false));
}

bool
Request::isValidVersion(const std::string& version)
{
    if (version.compare("HTTP/1.1") == 0 || version.compare("HTTP/1.0") == 0)
        return (true);
    return (this->updateStatusCodeAndReturn("400", false));
}

bool
Request::isValidHeaders(std::string& key, std::string& value)
{
    if (key.empty() || value.empty() ||
        this->isValidSP(key) == false ||
        this->isDuplicatedHeader(key) == false)
        return (false);
    return (true);
}

bool
Request::isValidSP(std::string& str)
{
    if (str.find(" ") == std::string::npos)
        return (true);
    return (false);
}

bool
Request::isDuplicatedHeader(std::string& key)
{
    if (this->_headers.find(key) == this->_headers.end())
        return (true);
    return (false);
}

bool
Request::isCarriegeReturnTrimmed()
{
    return (this->getCarriegeReturnTrimmed());
}
