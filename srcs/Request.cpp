#include "Request.hpp"

/*============================================================================*/
/****************************  Static variables  ******************************/
/*============================================================================*/

/*============================================================================*/
/******************************  Constructor  *********************************/
/*============================================================================*/

Request::Request()
: _request_method(""), _request_uri(""), _request_version(""),
 _request_protocol(""),
_request_bodies(""), _request_transfer_type(""), _status_code("") {}

Request::Request(const Request& other)
: _request_method(other._request_method), _request_uri(other._request_uri), 
_request_version(other._request_version), _request_headers(other._request_headers),
_request_protocol(other._request_protocol), _request_bodies(other._request_bodies), 
_request_transfer_type(other._request_transfer_type), _status_code(other._status_code) {}

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

void Request::setRequestMethod(const std::string& method)
{
    this->_request_method = method;
}

void Request::setRequestUri(const std::string& uri)
{
    this->_request_uri = uri;
}

void Request::setRequestVersion(const std::string& version)
{
    this->_request_version = version;
}

//TODO: insert를 하기 때문에 중복된 헤더가 키로 들어올 때 무시된다. 만약에 처음 삽입된 밸류에 문제가 있으면 그것이 그냥 작동하는 것..
void Request::setRequestHeaders(std::map<std::string, std::string>& headers)
{
    for (auto& h : headers)
        this->_request_headers.insert(make_pair(h.first, h.second));
}

void Request::setRequestHeaders(const std::string& key, const std::string& value)
{
    this->_request_headers[key] = value;
}

void Request::setRequestProtocol(const std::string& protocol)
{
    this->_request_protocol = protocol;
}

void Request::setRequestBodies(const std::string& req_message)
{
    this->_request_bodies = req_message;
}

void Request::setRequestTransferType(const std::string& transfer_type)
{
    this->_request_transfer_type = transfer_type;
}

void Request::setStatusCode(const std::string& status_code)
{
    this->_status_code = status_code;
}

/*============================================================================*/
/******************************  Exception  ***********************************/
/*============================================================================*/

/*============================================================================*/
/*********************************  Util  *************************************/
/*============================================================================*/


// //TODO: Server에서 getRequest() 함수를 실행시킬 때 먼저 req_message에 read버퍼를 모두 담아주어야 한다.
// Request Server::receiveRequest(Request& request)
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
//         if (parseRequest(req_message) == false)
//             return (false);
//         else
//             return (true);
//     }
// }


//NOTE: Request = Server::getRequest() 의 호출을 받고 루프 안에서 read 함수를 실행시킨다.
//NOTE: message body가 있다면 octets의 양이 message_body_length와 같을 때까지 읽거나 커넥션을 닫는다.
//NOTE: HTTP 메세지를 octet sequence로 인코딩해야 하며 그것은 US-ASCII로 이루어진다.

bool Request::parseRequest(std::string& req_message)
{
    std::string line;

    if (ft::substr(line, req_message, "\r\n") == false)
    {
        Request::setStatusCode("400");
        return (false);
    }
    else
    {
        if (parseRequestLine(line) == false)
            return (false);
    }

    if (ft::substr(line, req_message, "\r\n\r\n") == false)
        return (false);
    else
    {
        if (parseRequestHeaders(line) == false)
            return (false);
    }

    if (this->_request_headers.find("Transfer-Encoding") != this->_request_headers.end())
    {
        if (this->_request_headers["Transfer-Encoding"] == "chunked")
            return (parseChunkedBody(req_message));
    }

    if (ft::substr(line, req_message, "\r\n\r\n") == false)
        return (false);
    else
        parseRequestBodies(line);
    return (true);
}

bool Request::parseRequestLine(std::string& req_message)
{
    std::vector<std::string> request_line = ft::split(req_message, " ");

    setRequestMethod(request_line[0]);
    setRequestUri(request_line[1]);
    setRequestVersion(request_line[2]);
    return (true);
}

bool Request::parseRequestHeaders(std::string& req_message)
{
    std::string key;
    std::string value;
    std::map<std::string, std::string> headers;
    std::string line;

    while (ft::substr(line, req_message, "\r\n") == true && !req_message.empty())
    {
        //TODO: key에 공백이 있는지 체크해야함. value 역시 유효성 검사 해야함.
        if (ft::substr(key, line, ":") == false)
            return (false);
        value = ft::ltrim(line, " ");

        headers[key] = value;
        this->setRequestHeaders(key, value);
    }
    if (ft::substr(key, line, ":") == false)
        return (false);
    value = ft::ltrim(line, " ");

    headers[key] = value;
    this->setRequestHeaders(key, value);

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
            return (true);
        else if (line_len != -1)
        {
            if (ft::substr(line, req_message, "\r\n") == true && !req_message.empty())
                this->_request_bodies += line.substr(0, line_len) + "\r\n";
            else
                return (false);
        }
        else
            return (false);
    }
    return (true);
}

void Request::parseRequestBodies(std::string& req_message)
{
    this->setRequestBodies(req_message);
}
