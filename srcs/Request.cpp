#include "Request.hpp"

/*============================================================================*/
/****************************  Static variables  ******************************/
/*============================================================================*/

/*============================================================================*/
/******************************  Constructor  *********************************/
/*============================================================================*/

Request::Request()
{
}

// Request::Request(const Request& object)
// {
// }

/*============================================================================*/
/******************************  Destructor  **********************************/
/*============================================================================*/

Request::~Request()
{
}

/*============================================================================*/
/*******************************  Overload  ***********************************/
/*============================================================================*/

// Request& Request::operator=(const Request& object)
// {
//     return (*this);
// }
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

std::string Request::getRequestBodies()
{
    return (this->_request_bodies);
}

/*============================================================================*/
/********************************  Setter  ************************************/
/*============================================================================*/

void Request::setRequestMethod(std::string method)
{
    this->_request_method = method;
}

void Request::setRequestUri(std::string uri)
{
    this->_request_uri = uri;
}

void Request::setRequestVersion(std::string version)
{
    this->_request_version = version;
}

void Request::setRequestHeaders(std::map<std::string, std::string> headers)
{
    for (auto& h : headers)
    {
        this->_request_headers.insert(make_pair(h.first, h.second));
    }
}

void Request::setRequestBodies(std::string req_message)
{
    this->_request_bodies = req_message;
}

/*============================================================================*/
/******************************  Exception  ***********************************/
/*============================================================================*/

/*============================================================================*/
/*********************************  Util  *************************************/
/*============================================================================*/


//TODO: Server에서 getRequest() 함수를 실행시킬 때 먼저 req_message에 read버퍼를 모두 담아주어야 한다.
// Request Server::getRequest()
// {
//     int bytes;
//     std::string req_message;

//     bytes = 0;
//     while ((bytes = read(this->getFd(), buf, BUFFER_SIZE)) > 0)
//     {
//         req_message += buf;
//     }
//     if (bytes == 0)
//     {
//         parseRequest(req_message);
//     }
// }


//NOTE: Request = Server::getRequest() 의 호출을 받고 루프 안에서 read 함수를 실행시킨다.
//NOTE: message body가 있다면 octets의 양이 message_body_length와 같을 때까지 읽거나 커넥션을 닫는다.
//NOTE: HTTP 메세지를 octet sequence로 인코딩해야 하며 그것은 US-ASCII로 이루어진다.

void Request::parseRequest(std::string req_message)
{
    std::string line;

    line = ft::getLine(req_message, "\r\n");
    //NOTE: req_message가 empty면 delimeter find에 실패했다는 것
    if (req_message.empty())
    {
        throw "Unvalid Request Line";
    }
    else
        parseRequestLine(line);

    line = ft::getLine(req_message, "\r\n\r\n");
    if (req_message.empty())
    {
        if (line.find("\r\n") && line.find(":") != std::string::npos)
            parseRequestHeaders(line);
        else if (line.find("\r\n"))
        {
            parseRequestBodies(line);
            return ;
        }
        else
            throw "Unvalid Request Headers";
    }
    else
        parseRequestHeaders(line);

    line = ft::getLine(req_message, "\r\n\r\n");
    if (req_message.empty())
    {
        if (line.find("\r\n"))
            parseRequestBodies(line);
        else
            throw "Unvalid Request Bodies";
    }
    else
        parseRequestBodies(line);
}

void Request::parseRequestLine(std::string req_message)
{
    std::vector<std::string> request_line = ft::split(req_message, " ");
    if (request_line.size() != 3)
        throw "Request Line is Invalid";

    setRequestMethod(request_line[0]);
    setRequestUri(request_line[1]);
    setRequestVersion(request_line[2]);
}

void Request::parseRequestHeaders(std::string req_message)
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

        //TODO: value 예외처리 변경해야함, 예외처리 모듈화 시켜야함.
        // if (key.find(" ") != std::string::npos || value.find(":") != std::string::npos)
        //     return ;
            // throw "Unvalid Request Header Fields";

        headers[key] = value;
        Request::setRequestHeaders(headers);
    }
    key = ft::getLine(line, ":");
    value = ft::ltrim(line, " ");
    headers[key] = value;
    Request::setRequestHeaders(headers);
}

void Request::parseRequestBodies(std::string req_message)
{
    Request::setRequestBodies(req_message);
}


// ***************** Valid check 부분
//TODO: 스타트라인과 첫번째 헤더 필드 사이에 공백이 있으면 안됨.

//NOTE: HTTP-Message Format
/*
** HTTP-message   = start-line
**                 *( header-field CRLF )
**                 CRLF
**                 [ message-body ]
*/