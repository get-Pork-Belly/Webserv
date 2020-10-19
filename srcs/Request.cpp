#include "Request.hpp"

/*============================================================================*/
/****************************  Static variables  ******************************/
/*============================================================================*/

/*============================================================================*/
/******************************  Constructor  *********************************/
/*============================================================================*/

Request::Request()
: _request_method(""), _request_uri(""), _request_version(""), _request_protocol(""), _request_bodies(""), _request_transfer_type(""), _status_code("") {}

/*============================================================================*/
/******************************  Destructor  **********************************/
/*============================================================================*/

Request::~Request() {}

/*============================================================================*/
/********************************  Getter  ************************************/
/*============================================================================*/

std::string Request::getRequestMethod()
{
    return (this->_request_method);
}

std::string Request::getRequestUri()
{
    return (this->_request_uri);
}

std::string Request::getRequestVersion()
{
    return (this->_request_version);
}

std::map<std::string, std::string> Request::getRequestHeaders()
{
    return (this->_request_headers);
}

std::string Request::getRequestProtocol()
{
    return (this->_request_protocol);
}

std::string Request::getRequestBodies()
{
    return (this->_request_bodies);
}

std::string Request::getRequestTransferType()
{
    return (this->_request_transfer_type);
}

std::string Request::getStatusCode()
{
    return (this->_status_code);
}

/*============================================================================*/
/********************************  Setter  ************************************/
/*============================================================================*/

void Request::setRequestMethod(const std::string &method)
{
    this->_request_method = method;
}

void Request::setRequestUri(const std::string &uri)
{
    this->_request_uri = uri;
}

void Request::setRequestVersion(const std::string &version)
{
    this->_request_version = version;
}

//TODO: insert를 하기 때문에 중복된 헤더가 키로 들어올 때 무시된다. 만약에 처음 삽입된 밸류에 문제가 있으면 그것이 그냥 작동하는 것..
void Request::setRequestHeaders(std::map<std::string, std::string> &headers)
{

    for (auto& h : headers)
    {
        this->_request_headers.insert(make_pair(h.first, h.second));
    }
}

void Request::setRequestHeaders(const std::string &key, const std::string &value)
{
    this->_request_headers[key] = value;
}

void Request::setRequestProtocol(const std::string &protocol)
{
    this->_request_protocol = protocol;
}

void Request::setRequestBodies(const std::string &req_message)
{
    this->_request_bodies = req_message;
}

void Request::setRequestTransferType(const std::string &transfer_type)
{
    this->_request_transfer_type = transfer_type;
}

void Request::setStatusCode(const std::string &status_code)
{
    this->_status_code = status_code;
}

/*============================================================================*/
/******************************  Exception  ***********************************/
/*============================================================================*/

/*============================================================================*/
/*********************************  Util  *************************************/
/*============================================================================*/


//TODO: Server에서 getRequest() 함수를 실행시킬 때 먼저 req_message에 read버퍼를 모두 담아주어야 한다.
Request Server::receiveRequest(Request &request)
{
    int bytes;
    std::string req_message;

    bytes = 0;
    while ((bytes = read(this->getFd(), buf, BUFFER_SIZE)) > 0)
    {
        req_message += buf;
    }
    if (bytes == 0)
    {
        if (parseRequest(req_message) == false)
            return (false);
        else
            return (true);
    }
}


//NOTE: Request = Server::getRequest() 의 호출을 받고 루프 안에서 read 함수를 실행시킨다.
//NOTE: message body가 있다면 octets의 양이 message_body_length와 같을 때까지 읽거나 커넥션을 닫는다.
//NOTE: HTTP 메세지를 octet sequence로 인코딩해야 하며 그것은 US-ASCII로 이루어진다.

bool Request::parseRequest(std::string req_message)
{
    std::string line;

    line = ft::getLine(req_message, "\r\n");
    //NOTE: req_message가 empty면 delimeter find에 실패했다는 것
    if (req_message.empty())
    {
        Request::setStatusCode("400");
        return (false);
    }
    else
    {
        if (parseRequestLine(line) == false)
            return (false);
    }

    line = ft::getLine(req_message, "\r\n\r\n");
    if (req_message.empty())
    {
        if (line.find("\r\n") && line.find(":") != std::string::npos)
        {
            if (parseRequestHeaders(line) == false)
                return (false);
        }
        else if (line.find("\r\n"))
        {
            parseRequestBodies(line);
            return (false);
        }
        else
            return (false);
    }
    else
    {
        if (parseRequestHeaders(line) == false)
            return (false);
    }

    line = ft::getLine(req_message, "\r\n\r\n");
    if (req_message.empty())
    {
        if (line.find("\r\n"))
            parseRequestBodies(line);
        else
            return (false);
    }
    else
        parseRequestBodies(line);

    return (true);
}

bool Request::parseRequestLine(std::string &req_message)
{
    std::vector<std::string> request_line = ft::split(req_message, " ");
    
    if (isValidRequestLine(request_line) == false)
        return (false);

    setRequestMethod(request_line[0]);
    setRequestUri(request_line[1]);
    setRequestVersion(request_line[2]);
    return (true);
}

bool Request::parseRequestHeaders(std::string &req_message)
{
    std::string key;
    std::string value;
    std::map<std::string, std::string> headers;
    std::string line;

    while (!(line = ft::getLine(req_message, "\r\n")).empty() && !req_message.empty())
    {
        //TODO: key에 공백이 있는지 체크해야함. value 역시 유효성 검사 해야함.
        key = ft::getLine(line, ":");
        value = ft::ltrim(line, " ");

        if (this->isValidRequestHeaders(key, value) == false)
            return (false);

        if (this->isDuplicated(key) == false)
        {
            return (false);
        }

        headers[key] = value;
        // this->setRequestHeaders(headers);
        this->setRequestHeaders(key, value);
    }
    key = ft::getLine(line, ":");
    value = ft::ltrim(line, " ");

    if (this->isDuplicated(key) == false)
    {
        return (false);
    }

    headers[key] = value;
    // this->setRequestHeaders(headers);
    this->setRequestHeaders(key, value);


    return (true);
}

void Request::parseRequestBodies(std::string &req_message)
{
    Request::setRequestBodies(req_message);
}


/*============================================================================*/
/*****************************  Valid Check  **********************************/
/*============================================================================*/

//TODO: 스타트라인과 첫번째 헤더 필드 사이에 공백이 있으면 안됨.

//NOTE: HTTP-Message Format
/*
** HTTP-message   = start-line
**                 *( header-field CRLF )
**                 CRLF
**                 [ message-body ]
*/

bool Request::isValidRequestLine(std::vector<std::string> &request_line)
{
    if (request_line.size() != 3 ||
        Request::isValidRequestMethod(request_line[0]) == false ||
        Request::isValidRequestUri(request_line[1]) == false ||
        Request::isValidRequestVersion(request_line[2]) == false)
        return (false);
    return (true);
}

bool Request::isValidRequestMethod(const std::string &method)
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
    return (false);
}

//TODO: uri 유효성 검사 부분 더 알아보기.
bool Request::isValidRequestUri(const std::string &uri)
{
    if (uri.at(0) == '/' || uri.at(0) == 'w')
        return (true);
    return (false);
}

bool Request::isValidRequestVersion(const std::string &version)
{
    if (version.compare("HTTP/1.1") == 0)
        return (true);
    return (false);
}

//TODO: Request Headers 유효성 검사 부분 더 알아보기.
bool Request::isValidRequestHeaders(std::string &key, std::string &value)
{
    if (key.empty() || value.empty())
        return (false);

    //TODO: 헤더들이 \r\n으로 구분되어 있지 않을 때 예외처리 해야 함. 가장 좋은 방법은 value안에 key값이 존재하는지 파악하는 것일 듯 하다..
    // if ()

    // if (this->isDuplicated(this->_request_headers, key) == false)
    //     return (false);

    // if (this->isValidSP(key) == false)
    //     return (false);
    
    // if (this->isValidRequestHeaderFields(key) == false)
    //     return (false);

    return (true);
}

bool Request::isValidRequestHeaderFields(std::string &key)
{
    if (key.compare("Accept-Charsets") != 0 &&
        key.compare("Accept-Language") != 0 &&
        key.compare("Allow") != 0 &&
        key.compare("Authorization") != 0 &&
        key.compare("Content-Language") != 0 &&
        key.compare("Content-Length") != 0 &&
        key.compare("Content-Location") != 0 &&
        key.compare("Content-Type") != 0 &&
        key.compare("Date") != 0 &&
        key.compare("Host") != 0 &&
        key.compare("Last-Modified") != 0 &&
        key.compare("Location") != 0 &&
        key.compare("Referer") != 0 &&
        key.compare("Retry-After") != 0 &&
        key.compare("Server") != 0 &&
        key.compare("Transfer-Encoding") != 0 &&
        key.compare("User-Agent") != 0 &&
        key.compare("WWW-Authenticate") != 0)
        return (false);
    return (true);
}

//NOTE: 헤더의 key에 공백 없으면 true
bool Request::isValidSP(std::string &str)
{
    if (str.find(" ") == std::string::npos)
        return (true);
    return (false);
}

//TODO: 중복검사 더 좋은 거 찾기. 일단 아래코드는 동작 안함..
bool Request::isDuplicated(std::string &key)
{
    if (this->_request_headers.find(key) == this->_request_headers.end())
        return (true);
    return (false);
}