#include "Request.hpp"
#include "Log.hpp"

/*============================================================================*/
/****************************  Static variables  ******************************/
/*============================================================================*/

/*============================================================================*/
/******************************  Constructor  *********************************/
/*============================================================================*/

Request::Request()
: _method(""), _uri(""), _version(""),
_protocol(""), _body(""), _chunked_body(""), _status_code("200"),
_info(ReqInfo::READY), _is_buffer_left(false),
_ip_address(""), _transfered_body_size(0) {}

Request::Request(const Request& other)
: _method(other._method), _uri(other._uri), 
_version(other._version), _headers(other._headers),
_protocol(other._protocol), _body(other._body), _chunked_body(other._chunked_body),
_status_code(other._status_code), _info(other._info),
_is_buffer_left(other._is_buffer_left), _ip_address(other._ip_address),
_transfered_body_size(other._transfered_body_size) {}

Request&
Request::operator=(const Request& other)
{
    this->_method= other._method;
    this->_uri = other._uri;
    this->_version = other._version;
    this->_headers = other._headers;
    this->_protocol = other._protocol;
    this->_body = other._body;
    this->_chunked_body = other._chunked_body;
    this->_status_code = other._status_code;
    this->_info = other._info;
    this->_is_buffer_left = other._is_buffer_left;
    this->_ip_address = other._ip_address;
    this->_transfered_body_size = other._transfered_body_size;
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

const std::string&
Request::getChunkedBody() const
{
    return (this->_chunked_body);
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
Request::setChunkedBody(const std::string& chunked_body)
{
    this->_chunked_body = chunked_body;
}

/*============================================================================*/
/******************************  Exception  ***********************************/
/*============================================================================*/

Request::RequestFormatException::RequestFormatException(Request& req, const std::string& status_code)
: _msg("RequestFormatException: Invalid Request Format: " + status_code), _req(req) 
{
    this->_req.setStatusCode(status_code);
}

Request::RequestFormatException::RequestFormatException(Request& req)
: _msg("RequestFormatException: Invalid Request Format: "), _req(req) {}

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
    Log::trace("> updateReqInfo");
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
    Log::trace("< updateReqInfo");
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
    return ((this->getReqInfo() == ReqInfo::COMPLETE) ? false : !isNormalBody());
}


bool
Request::isContentLeftInBuffer() const
{
    return (this->getIsBufferLeft());
}


bool
Request::updateStatusCodeAndReturn(const std::string& status_code, const bool& ret)
{
    this->setStatusCode(status_code);
    return (ret);
}

void
Request::parseRequestWithoutBody(char* buf)
{
    Log::trace("> parseRequestWithoutBody");
    std::string line;
    std::string req_message(buf);

    if (ft::substr(line, req_message, "\r\n") == false)
        throw (RequestFormatException(*this, "400"));
    else
    {
        if (this->parseRequestLine(line) == false)
            throw (RequestFormatException(*this));
    }
    if (ft::substr(line, req_message, "\r\n\r\n") == false)
        throw (RequestFormatException(*this, "400"));
    else
    {
        if (this->parseHeaders(line) == false)
            throw (RequestFormatException(*this));
    }
    this->updateReqInfo();
    Log::trace("< parseRequestWithoutBody");
}

bool
Request::parseRequestLine(std::string& req_message)
{
    Log::trace("> parseRequestLine");
    std::vector<std::string> request_line = ft::split(req_message, " ");
    
    if (this->isValidLine(request_line) == false)
        return (false);
    this->setMethod(request_line[0]);
    this->setUri(request_line[1]);
    this->setVersion(request_line[2]);
    Log::trace("< parseRequestLine");
    return (true);
}

bool
Request::parseHeaders(std::string& req_message)
{
    Log::trace("> parseHeaders");
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
    Log::trace("< parseHeaders");
    return (true);
}

void
Request::parseChunkedBody(const std::string& body)
{
    int line_len;
    std::string line;
    std::string req_message(body);

    if (req_message.find("\r\n") == std::string::npos)
    {
        this->setStatusCode("400");
        throw (RequestFormatException(*this));
    }
    while (ft::substr(line, req_message, "\r\n") && !req_message.empty())
    {
        std::cout << "linelen: " << line_len << std::endl;
        line_len = ft::stoiHex(line);
        if (line_len == 0)
        {
            this->setReqInfo(ReqInfo::COMPLETE);
            return ;
        }
        else if (line_len != -1)
        {
            if (ft::substr(line, req_message, "\r\n") && !req_message.empty())
                this->_body += line.substr(0, line_len) + "\r\n";
            else
            {
                this->setStatusCode("400");
                throw (RequestFormatException(*this));
            }
        }
        else
        {
            this->setStatusCode("400");
            throw (RequestFormatException(*this));
        }
    }
}

void
Request::appendBody(char* buf, int bytes)
{
    this->setBody(this->getBody() + std::string(buf, bytes));
}

void
Request::appendChunkedBody(char* buf, size_t bytes)
{
    this->setChunkedBody(this->getChunkedBody() + std::string(buf, bytes));
}

void
Request::init()
{
    Request temp;
    *this = temp;
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
        method.compare("OPTION") == 0 ||
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
