#include "Request.hpp"

/*============================================================================*/
/****************************  Static variables  ******************************/
/*============================================================================*/

/*============================================================================*/
/******************************  Constructor  *********************************/
/*============================================================================*/

Request::Request()
: _request_method(""), _request_uri(""), _request_version(""),
 _request_protocol(""), _request_bodies(""), _request_transfer_type(""), 
 _status_code("") {}

Request::Request(const Request& other)
: _request_method(other._request_method), _request_uri(other._request_uri), 
_request_version(other._request_version), _request_headers(other._request_headers),
_request_protocol(other._request_protocol), _request_bodies(other._request_bodies), 
_request_transfer_type(other._request_transfer_type), _status_code(other._status_code) {}

Request&
Request::operator=(const Request& other)
{
    this->_request_method= other._request_method;
    this->_request_uri = other._request_uri;
    this->_request_version = other._request_version;
    this->_request_headers = other._request_headers;
    this->_request_protocol = other._request_protocol;
    this->_request_bodies = other._request_bodies;
    this->_request_transfer_type = other._request_transfer_type;
    this->_status_code = other._status_code;
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
Request::getRequestMethod()
{
    return (this->_request_method);
}

std::string
Request::getRequestUri()
{
    return (this->_request_uri);
}

std::string
Request::getRequestVersion()
{
    return (this->_request_version);
}

std::map<std::string, std::string>
Request::getRequestHeaders()
{
    return (this->_request_headers);
}

std::string
Request::getRequestProtocol()
{
    return (this->_request_protocol);
}

std::string
Request::getRequestBodies()
{
    return (this->_request_bodies);
}

std::string
Request::getRequestTransferType()
{
    return (this->_request_transfer_type);
}

std::string
Request::getStatusCode()
{
    return (this->_status_code);
}

/*============================================================================*/
/********************************  Setter  ************************************/
/*============================================================================*/

void
Request::setRequestMethod(const std::string& method)
{
    this->_request_method = method;
}

void
Request::setRequestUri(const std::string& uri)
{
    this->_request_uri = uri;
}

void
Request::setRequestVersion(const std::string& version)
{
    this->_request_version = version;
}

//TODO: insert를 하기 때문에 중복된 헤더가 키로 들어올 때 무시된다. 만약에 처음 삽입된 밸류에 문제가 있으면 그것이 그냥 작동하는 것..
void
Request::setRequestHeaders(const std::string& key, const std::string& value)
{
    this->_request_headers[key] = value;
}

void
Request::setRequestProtocol(const std::string& protocol)
{
    this->_request_protocol = protocol;
}

void
Request::setRequestBodies(const std::string& req_message)
{
    this->_request_bodies = req_message;
}

void
Request::setRequestTransferType(const std::string& transfer_type)
{
    this->_request_transfer_type = transfer_type;
}

void
Request::setStatusCode(const std::string& status_code)
{
    this->_status_code = status_code;
}

/*============================================================================*/
/******************************  Exception  ***********************************/
/*============================================================================*/

/*============================================================================*/
/*********************************  Util  *************************************/
/*============================================================================*/

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
        if (parseRequestHeaders(line) == false)
        {
            this->setStatusCode("400");
            return (false);
        }
    }

    if (this->_request_headers.find("Transfer-Encoding") != this->_request_headers.end())
    {
        if (this->_request_headers["Transfer-Encoding"] == "chunked")
            return (parseChunkedBody(req_message));
    }
    return (parseRequestBodies(req_message));
}

bool
Request::parseRequestLine(std::string& req_message)
{
    std::vector<std::string> request_line = ft::split(req_message, " ");
    
    if (isValidRequestLine(request_line) == false)
    {
        this->setStatusCode("400");
        return (false);
    }

    setRequestMethod(request_line[0]);
    setRequestUri(request_line[1]);
    setRequestVersion(request_line[2]);
    return (true);
}

bool
Request::parseRequestHeaders(std::string& req_message)
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
        if (this->isValidRequestHeaders(key, value) == false)
        {
            this->setStatusCode("400");
            return (false);
        }
        this->setRequestHeaders(key, value);
    }
    if (ft::substr(key, line, ":") == false)
    {
        this->setStatusCode("400");
        return (false);
    }
    value = ft::ltrim(line, " ");
    if (this->isValidRequestHeaders(key, value) == false)
    {
        this->setStatusCode("400");
        return (false);
    }
    this->setRequestHeaders(key, value);

    return (true);
}

bool
Request::parseChunkedBody(std::string &req_message)
{
    int line_len;
    std::string line;

    //TODO: 라인에서 \r\n을 찾지 못하면 true로 종료됨. 이후 valid check 시 상태코드 업데이트하게끔 수정요망.
    while (ft::substr(line, req_message, "\r\n") == true && !req_message.empty())
    {
        line_len = ft::stoiHex(line);
        if (line_len == 0)
            return (true);
        else if (line_len != -1)
        {
            if (ft::substr(line, req_message, "\r\n") == true && !req_message.empty())
                this->_request_bodies += line.substr(0, line_len) + "\r\n";
            else
            {
                this->setStatusCode("400");
                return (false);
            }
        }
        else
        {
            this->setStatusCode("400");
            return (false);
        }
    }
    return (true);
}

bool
Request::parseRequestBodies(std::string& req_message)
{
    this->setRequestBodies(req_message);
    return (true);
}

/*============================================================================*/
/*****************************  Valid Check  **********************************/
/*============================================================================*/

//TODO: return false 일 경우 this->setStatusCode("PROPER STATUS_CODE"); 처리를 해주어야 합니다.

bool
Request::isValidRequestLine(std::vector<std::string>& request_line)
{
    if (request_line.size() != 3 ||
        this->isValidRequestMethod(request_line[0]) == false ||
        this->isValidRequestUri(request_line[1]) == false ||
        this->isValidRequestVersion(request_line[2]) == false)
    {
        this->setStatusCode("400");
        return (false);
    }
    return (true);
}

bool
Request::isValidRequestMethod(const std::string& method)
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
    this->setStatusCode("400");
    return (false);
}

//TODO: uri 유효성 검사 부분 더 알아보기.
bool
Request::isValidRequestUri(const std::string& uri)
{
    if (uri[0] == '/' || uri[0] == 'w')
        return (true);
    this->setStatusCode("400");
    return (false);
}

bool
Request::isValidRequestVersion(const std::string& version)
{
    if (version.compare("HTTP/1.1") == 0 || version.compare("HTTP/1.0") == 0)
        return (true);
    this->setStatusCode("400");
    return (false);
}

bool
Request::isValidRequestHeaders(std::string& key, std::string& value)
{
    if (key.empty() || value.empty() ||
        this->isValidSP(key) == false ||
        this->isDuplicated(key) == false)
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
Request::isDuplicated(std::string& key)
{
    if (this->_request_headers.find(key) == this->_request_headers.end())
        return (true);
    this->setStatusCode("400");
    return (false);
}
