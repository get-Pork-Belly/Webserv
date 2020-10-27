#include "Request.hpp"

/*============================================================================*/
/****************************  Static variables  ******************************/
/*============================================================================*/

/*============================================================================*/
/******************************  Constructor  *********************************/
/*============================================================================*/

Request::Request()
: _method(""), _uri(""), _version(""),
 _protocol(""), _bodies(""), _status_code(""), _info(ReqInfo::READY) {}

Request::Request(const Request& other)
: _method(other._method), _uri(other._uri), 
_version(other._version), _headers(other._headers),
_protocol(other._protocol), _bodies(other._bodies), 
_status_code(other._status_code), _info(other._info) {}

Request&
Request::operator=(const Request& other)
{
    this->_method= other._method;
    this->_uri = other._uri;
    this->_version = other._version;
    this->_headers = other._headers;
    this->_protocol = other._protocol;
    this->_bodies = other._bodies;
    this->_status_code = other._status_code;
    this->_info = other._info;
    return (*this);
}

/*============================================================================*/
/******************************  Destructor  **********************************/
/*============================================================================*/

Request::~Request() {}

/*============================================================================*/
/********************************  Getter  ************************************/
/*============================================================================*/

std::string
Request::getMethod() const
{
    return (this->_method);
}

const std::string&
Request::getUri()
{
    return (this->_uri);
}

std::string
Request::getVersion()
{
    return (this->_version);
}

std::map<std::string, std::string>
Request::getHeaders() const
{
    return (this->_headers);
}

std::string
Request::getProtocol()
{
    return (this->_protocol);
}

std::string
Request::getBodies()
{
    return (this->_bodies);
}

std::string
Request::getStatusCode()
{
    return (this->_status_code);
}

const ReqInfo&
Request::getReqInfo() const
{
    return (this->_info);
}

const size_t&
Request::getHeaderEndPos() const
{
    return (this->_header_end_pos);
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

//TODO: insert를 하기 때문에 중복된 헤더가 키로 들어올 때 무시된다. 만약에 처음 삽입된 밸류에 문제가 있으면 그것이 그냥 작동하는 것..
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
Request::setBodies(const std::string& req_message)
{
    this->_bodies = req_message;
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

/*============================================================================*/
/******************************  Exception  ***********************************/
/*============================================================================*/

/*============================================================================*/
/*********************************  Util  *************************************/
/*============================================================================*/

void
Request::updateReqInfo()
{
    if (this->getMethod() == "" && this->getUri() == "" && this->getVersion() == "")
        setReqInfo(ReqInfo::READY);
    else if (this->isBodyUnnecessary())
        setReqInfo(ReqInfo::COMPLETE);
    else if (this->isNormalBody())
        setReqInfo(ReqInfo::NORMAL_BODY);
    else if (this->isChunkedBody())
        setReqInfo(ReqInfo::CHUNKED_BODY);
}

bool
Request::isBodyUnnecessary() const
{
    std::string method = this->getMethod();
    if (method.compare("PUT") || method.compare("POST"))
        return (false);
    return (true);
}

bool
Request::isNormalBody() const
{
    if (this->getReqInfo() == ReqInfo::COMPLETE)
        return (false);

    std::map<std::string, std::string> headers = this->getHeaders();
    location_info::iterator it;
    it = headers.find("Transfer-Encoding");
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
Request::parseRequestWithoutBody(std::string& req_message)
{
    std::string line;

    if (ft::substr(line, req_message, "\r\n") == false)
    {
        this->setStatusCode("400");
        return (false);
    }
    else
    {
        if (parseRequestLine(line) == false)
        {
            this->setStatusCode("400");
            return (false);
        }
    }

    if (ft::substr(line, req_message, "\r\n\r\n") == false)
    {
        this->setStatusCode("400");
        return (false);
    }
    else
    {
        if (parseHeaders(line) == false)
        {
            this->setStatusCode("400");
            return (false);
        }
    }
    return (true);
}

bool
Request::parseRequest(std::string& req_message)
{
    std::string line;

    if (ft::substr(line, req_message, "\r\n") == false)
    {
        this->setStatusCode("400");
        return (false);
    }
    else
    {
        if (parseRequestLine(line) == false)
        {
            this->setStatusCode("400");
            return (false);
        }
    }

    if (ft::substr(line, req_message, "\r\n\r\n") == false)
    {
        this->setStatusCode("400");
        return (false);
    }
    else
    {
        if (parseHeaders(line) == false)
        {
            this->setStatusCode("400");
            return (false);
        }
    }

    if (this->_headers.find("Transfer-Encoding") != this->_headers.end())
    {
        if (this->_headers["Transfer-Encoding"] == "chunked")
            return (parseChunkedBody(req_message));
    }
    return (parseBodies(req_message));
}

bool
Request::parseRequestLine(std::string& req_message)
{
    std::vector<std::string> request_line = ft::split(req_message, " ");
    
    if (isValidLine(request_line) == false)
    {
        this->setStatusCode("400");
        return (false);
    }

    this->setMethod(request_line[0]);
    this->setUri(request_line[1]);
    this->setVersion(request_line[2]);
    return (true);
}

bool
Request::parseHeaders(std::string& req_message)
{
    std::string key;
    std::string value;
    std::string line;

    while (ft::substr(line, req_message, "\r\n") == true && !req_message.empty())
    {
        if (ft::substr(key, line, ":") == false)
        {
            this->setStatusCode("400");
            return (false);
        }
        value = ft::ltrim(line, " ");
        if (this->isValidHeaders(key, value) == false)
        {
            this->setStatusCode("400");
            return (false);
        }
        this->setHeaders(key, value);
    }
    if (ft::substr(key, line, ":") == false)
    {
        this->setStatusCode("400");
        return (false);
    }
    value = ft::ltrim(line, " ");
    if (this->isValidHeaders(key, value) == false)
    {
        this->setStatusCode("400");
        return (false);
    }
    this->setHeaders(key, value);

    return (true);
}

bool
Request::parseChunkedBody(std::string &req_message)
{
    int line_len;
    std::string line;

    while (ft::substr(line, req_message, "\r\n") == true && !req_message.empty())
    {
        line_len = ft::stoiHex(line);
        if (line_len == 0)
        {
            this->setReqInfo(ReqInfo::COMPLETE);
            return (true);
        }
        else if (line_len != -1)
        {
            if (ft::substr(line, req_message, "\r\n") == true && !req_message.empty())
                this->_bodies += line.substr(0, line_len) + "\r\n";
            else
                return (false);
        }
        else
            return (false);
    }


    //TODO: CRLF 못찾았을 때 처리해주기.
    return (true);
}

bool
Request::parseBodies(std::string& req_message)
{
    this->setBodies(req_message);
    this->setReqInfo(ReqInfo::COMPLETE);
    return (true);
}

void
Request::clear()
{
    this->_method = "";
    this->_uri = "";
    this->_version = "";
    this->_protocol = "";
    this->_status_code = "";
    this->_bodies = "";
    this->setReqInfo(ReqInfo::READY);
    // this->_info = ReqInfo::READY;
}

/*============================================================================*/
/*****************************  Valid Check  **********************************/
/*============================================================================*/

//TODO: return false 일 경우 this->setStatusCode("PROPER STATUS_CODE"); 처리를 해주어야 합니다.

bool
Request::isValidLine(std::vector<std::string>& request_line)
{
    if (request_line.size() != 3 ||
        this->isValidMethod(request_line[0]) == false ||
        this->isValidUri(request_line[1]) == false ||
        this->isValidVersion(request_line[2]) == false)
    {
        this->setStatusCode("400");
        return (false);
    }
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
    this->setStatusCode("501");
    return (false);
}

//TODO: uri 유효성 검사 부분 더 알아보기.
bool
Request::isValidUri(const std::string& uri)
{
    if (uri[0] == '/' || uri[0] == 'w')
        return (true);
    this->setStatusCode("400");
    return (false);
}

bool
Request::isValidVersion(const std::string& version)
{
    if (version.compare("HTTP/1.1") == 0 || version.compare("HTTP/1.0") == 0)
        return (true);
    this->setStatusCode("400");
    return (false);
}

bool
Request::isValidHeaders(std::string& key, std::string& value)
{
    if (key.empty() || value.empty() ||
        this->isValidSP(key) == false ||
        this->isDuplicatedHeader(key) == false)
    {
        this->setStatusCode("400");
        return (false);
    }

    //TODO: 헤더들이 \r\n으로 구분되어 있지 않을 때 예외처리 해야 함.

    //TODO: 아래 TODO와 관련하여 변경하여야 함.
    // if (this->isValidRequestHeaderFields(key) == false)
    //     return (false);

    return (true);
}

//TODO: 아래에 없는 Header Fields가 들어왔을 때 고려하여 주석처리해놓습니다.
// bool
// Request::isValidRequestHeaderFields(std::string& key)
// {
//     if (key.compare("Accept-Charsets") != 0 &&
//         key.compare("Accept-Language") != 0 &&
//         key.compare("Allow") != 0 &&
//         key.compare("Authorization") != 0 &&
//         key.compare("Content-Language") != 0 &&
//         key.compare("Content-Length") != 0 &&
//         key.compare("Content-Location") != 0 &&
//         key.compare("Content-Type") != 0 &&
//         key.compare("Date") != 0 &&
//         key.compare("Host") != 0 &&
//         key.compare("Last-Modified") != 0 &&
//         key.compare("Location") != 0 &&
//         key.compare("Referer") != 0 &&
//         key.compare("Retry-After") != 0 &&
//         key.compare("Server") != 0 &&
//         key.compare("Transfer-Encoding") != 0 &&
//         key.compare("User-Agent") != 0 &&
//         key.compare("WWW-Authenticate") != 0)
//         return (false);
//     return (true);
// }

bool
Request::isValidSP(std::string& str)
{
    if (str.find(" ") == std::string::npos)
        return (true);
    this->setStatusCode("400");
    return (false);
}

bool
Request::isDuplicatedHeader(std::string& key)
{
    if (this->_headers.find(key) == this->_headers.end())
        return (true);
    this->setStatusCode("400");
    return (false);
}
