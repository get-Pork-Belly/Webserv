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
_status_code(other._status_code), _info(other._info),
_is_buffer_left(false) {}

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
    this->_is_buffer_left = other._is_buffer_left;
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
Request::getUri() const
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

bool
Request::getIsBufferLeft() const
{
    return (this->_is_buffer_left);
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

void
Request::setIsBufferLeft(const bool& is_left_buffer)
{
    this->_is_buffer_left = is_left_buffer;
}

/*============================================================================*/
/******************************  Exception  ***********************************/
/*============================================================================*/

Request::RequestFormatException::RequestFormatException(Request& req, const std::string& status_code)
: _msg("RequestFormatException: Invalid Request Format: "), _req(req) 
{
    req.setStatusCode(status_code);
}

Request::RequestFormatException::RequestFormatException(Request& req)
: _msg("RequestFormatException: Invalid Request Format: "), _req(req) {}

std::string
Request::RequestFormatException::s_what() const throw()
{
    std::string tmp;
    tmp += this->_msg;
    tmp += this->_req.getStatusCode();
    std::cout<<"in reqformat except: "<<tmp<<std::endl;
    return (tmp);
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
    if (this->getReqInfo() == ReqInfo::COMPLETE)
        return ;
    if (this->getMethod() == "" && this->getUri() == "" && this->getVersion() == "")
        setReqInfo(ReqInfo::READY);
    else if (this->isBodyUnnecessary())
        setReqInfo(ReqInfo::MUST_CLEAR);
    else if (this->isNormalBody())
        setReqInfo(ReqInfo::NORMAL_BODY);
    else if (this->isChunkedBody())
        setReqInfo(ReqInfo::CHUNKED_BODY);
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
    std::string line;
    std::string req_message(buf);

    if (ft::substr(line, req_message, "\r\n") == false)
        throw (RequestFormatException(*this, "400"));
    else
    {
        if (parseRequestLine(line) == false)
            throw (RequestFormatException(*this));
    }
    if (ft::substr(line, req_message, "\r\n\r\n") == false)
        throw (RequestFormatException(*this, "400"));
    else
    {
        if (parseHeaders(line) == false)
            throw (RequestFormatException(*this));
    }
    this->updateReqInfo();
}

bool
Request::parseRequestLine(std::string& req_message)
{
    std::vector<std::string> request_line = ft::split(req_message, " ");
    
    if (isValidLine(request_line) == false)
        return (false);
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

    while (ft::substr(line, req_message, "\r\n") && !req_message.empty())
    {
        if (ft::substr(key, line, ":") == false)
            return (updateStatusCodeAndReturn("400", false));
        value = ft::ltrim(line, " ");
        if (this->isValidHeaders(key, value) == false)
            return (updateStatusCodeAndReturn("400", false));
        this->setHeaders(key, value);
    }
    if (ft::substr(key, line, ":") == false)
        return (updateStatusCodeAndReturn("400", false));
    value = ft::ltrim(line, " ");
    if (this->isValidHeaders(key, value) == false)
        return (updateStatusCodeAndReturn("400", false));
    this->setHeaders(key, value);
    return (true);
}

void
Request::parseChunkedBody(char* buf)
{
    int line_len;
    std::string line;
    std::string req_message(buf);

    if (req_message.find("\r\n") == std::string::npos)
    {
        this->setStatusCode("400");
        throw (RequestFormatException(*this));
    }
    while (ft::substr(line, req_message, "\r\n") && !req_message.empty())
    {
        line_len = ft::stoiHex(line);
        if (line_len == 0)
        {
            this->setReqInfo(ReqInfo::COMPLETE);
            return ;
        }
        else if (line_len != -1)
        {
            if (ft::substr(line, req_message, "\r\n") && !req_message.empty())
                this->_bodies += line.substr(0, line_len) + "\r\n";
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
Request::parseNormalBodies(char* buf)
{
    std::string normal_body(buf);
    this->setBodies(normal_body);
    this->setReqInfo(ReqInfo::COMPLETE);
}

void
Request::clear()
{
    this->_method = "";
    this->_uri = "";
    this->_version = "";
    this->_protocol = "";
    this->_headers = {{"default", "default"}};
    this->_status_code = "";
    this->_bodies = "";
    this->_is_buffer_left = false;
    this->setReqInfo(ReqInfo::READY);
}

int
Request::getContentLength()
{
    location_info::iterator it = this->_headers.find("Content-Length");
    if (it == this->_headers.end())
        throw "Invalid NORMAL_BODY";
        // throw (NoContentLengthException());
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
    return (updateStatusCodeAndReturn("501", false));
}

//TODO: uri 유효성 검사 부분 더 알아보기.
bool
Request::isValidUri(const std::string& uri)
{
    if (uri[0] == '/' || uri[0] == 'w')
        return (true);
    return (updateStatusCodeAndReturn("400", false));
}

bool
Request::isValidVersion(const std::string& version)
{
    if (version.compare("HTTP/1.1") == 0 || version.compare("HTTP/1.0") == 0)
        return (true);
    return (updateStatusCodeAndReturn("400", false));
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
