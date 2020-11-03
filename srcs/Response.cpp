#include "Response.hpp"
#include "Server.hpp"
#include "PageGenerator.hpp"
#include "Log.hpp"

/*============================================================================*/
/****************************  Static variables  ******************************/
/*============================================================================*/

/*============================================================================*/
/******************************  Constructor  *********************************/
/*============================================================================*/

Response::Response()
: _status_code("200"), _transfer_type(""), _clients(""), _body("")
{
    this->_headers = { {"", ""} };
    ft::memset(&this->_file_info, 0, sizeof(this->_file_info));
    this->initStatusCodeTable();
}

Response::Response(const Response& other)
: _status_code(other._status_code),  _headers(other._headers),
_transfer_type(other._transfer_type), _clients(other._clients),
_status_code_table(other._status_code_table), _body(other._body) {}

/*============================================================================*/
/******************************  Destructor  **********************************/
/*============================================================================*/

Response::~Response()
{
}

/*============================================================================*/
/*******************************  Overload  ***********************************/
/*============================================================================*/

Response&
Response::operator=(const Response& rhs)
{
    this->_status_code = rhs._status_code;
    this->_headers = rhs._headers;
    this->_transfer_type = rhs._transfer_type;
    this->_clients = rhs._clients;
    this->_body= rhs._body;
    this->_status_code_table = rhs._status_code_table;
    return (*this);
}

/*============================================================================*/
/********************************  Getter  ************************************/
/*============================================================================*/

std::string 
Response::getStatusCode() const
{
    return (this->_status_code);
}

std::string
Response::getStatusMessage(const std::string& code)
{
    return (this->_status_code_table[code]);
}

const location_info&
Response::getLocationInfo() const
{
    return (this->_location_info);
}

const std::string&
Response::getRoute() const
{
    return (this->_route);
}

const std::string&
Response::getResourceAbsPath() const
{
    return (this->_resource_abs_path);
}

const std::string&
Response::getDirectoryEntry() const
{
    return (this->_directory_entry);
}

const struct stat&
Response::getFileInfo() const
{
    return (this->_file_info);
}

const ResType&
Response::getResourceType() const
{
    return (this->_resource_type);
}

const std::string&
Response::getBody() const
{
    return (this->_body);
}

int
Response::getCgiPipeFd() const
{
    return (this->_cgi_pipe[1]);
}

/*============================================================================*/
/********************************  Setter  ************************************/
/*============================================================================*/

void
Response::setStatusCode(const std::string& status_code)
{
    this->_status_code = status_code;
}

void
Response::setResourceAbsPath(const std::string& path)
{
    this->_resource_abs_path = path;
}

void
Response::setDirectoryEntry(DIR* dir_ptr)
{
    struct dirent* entry = NULL;
    while ((entry = readdir(dir_ptr)) != NULL)
    {
        this->_directory_entry += entry->d_name;
        if (entry->d_type == 4)
            this->_directory_entry += "/";
        this->_directory_entry += " ";
    }
}

void
Response::setFileInfo(const struct stat& file_info)
{
    this->_file_info = file_info;
}

void
Response::setResourceType(const ResType& resource_type)
{
    this->_resource_type = resource_type;
}

void
Response::setBody(const std::string& body)
{
    this->_body = body;
}

/*============================================================================*/
/******************************  Exception  ***********************************/
/*============================================================================*/

Response::CannotOpenCgiPipeException::CannotOpenCgiPipeException(Response& response)
: _response(response)
{
    this->_response.setStatusCode("500");
}

const char*
Response::CannotOpenCgiPipeException::what() const throw()
{
    return ("[CODE 500] Cannot Open Cgi Pipe.");
}

/*============================================================================*/
/*********************************  Util  *************************************/
/*============================================================================*/

void
Response::init()
{
    this->_status_code = "200";
    this->_headers = { {"", ""} };
    this->_transfer_type = "";
    this->_clients = "";
    this->_body = "";
    this->_location_info = { {"", ""} };
    this->_resource_abs_path = "";
    this->_route = "";
    this->_directory_entry = "";
}

void
Response::initStatusCodeTable()
{
    this->_status_code_table = {
        {"100", "Continue"},
        {"101", "Switching Protocols"},
        {"102", "Processing"},
        {"200", "OK"},
        {"201", "Created"},
        {"202", "Accepted"},
        {"203", "Non-authoritative Information"},
        {"204", "No Content"},
        {"205", "Reset Content"},
        {"206", "Partial Content"},
        {"207", "Multi-Status"},
        {"208", "Already Reported"},
        {"226", "IM Used"},
        {"300", "Multiple Choices"},
        {"301", "Moved Permanently"},
        {"302", "Found"},
        {"303", "See Other"},
        {"304", "Not Modified"},
        {"305", "Use Proxy"},
        {"307", "Temporary Redirect"},
        {"308", "Permanent Redirect"},
        {"400", "Bad Request"},
        {"401", "Unauthorized"},
        {"402", "Payment Required"},
        {"403", "Forbidden"},
        {"404", "Not found"},
        {"405", "Method Not Allowed"},
        {"406", "Not Acceptable"},
        {"407", "Proxy Authentication Required"},
        {"408", "Required Timeout"},
        {"409", "Conflict"},
        {"410", "Gone"},
        {"411", "Length Required"},
        {"412", "Precondition Failed"},
        {"413", "Request Entity Too Large"},
        {"414", "Request URI Too Long"},
        {"415", "Unsupported Media Type"},
        {"416", "Requested Range Not Satisfiable"},
        {"417", "Expectation Failed"},
        {"418", "IM_A_TEAPOT"},
        {"500", "Internal Server Error"},
        {"501", "Not Implemented"},
        {"502", "Bad Gateway"},
        {"503", "Service Unavailable"},
        {"504", "Gateway Timeout"},
        {"505", "HTTP Version Not Supported"},
        {"506", "Variant Also Negotiates"},
        {"507", "Insufficient Storage"},
        {"508", "Loop Detected"},
        {"510", "Not Extened"},
        {"511", "Network Authentication Required"},
        {"599", "Network Connect Timeout Error"},
    };
}

//TODO: Response에 상태코드 세팅하게 변경하기.
void
Response::applyAndCheckRequest(Request& request, Server* server)
{
    Log::trace("> applyAndCheckRequest");
    if (this->setRouteAndLocationInfo(request.getUri(), server))
    {
        if (this->isLimitExceptInLocation() && this->isAllowedMethod(request.getMethod()) == false)
            this->setStatusCode("405");
    }
    Log::trace("< applyAndCheckRequest");
}

bool
Response::setRouteAndLocationInfo(const std::string& uri, Server* server)
{
    Log::trace("> setRouteAndLocationInfo");
    std::map<std::string, location_info> location_config = server->getLocationConfig();
    std::string route;

    if (uri.length() == 1)
    {
        if (location_config.find("/") != location_config.end())
        {
            this->_route = uri;
            this->_location_info = location_config["/"];
            return (true);
        }
        return (false);
    }
    size_t index = (uri[uri.length() - 1] == '/') ? uri.length() : uri.length() + 1;
    while ((index = uri.find_last_of("/", index - 1)) != std::string::npos)
    {
        route = uri.substr(0, index);
        if (location_config.find(route) != location_config.end())
        {
            this->_route = route;
            this->_location_info = location_config[route];
            return (true);
        }
        if (index == 0)
        {
            this->_route = "/";
            this->_location_info = location_config["/"];
            break ;
        }
    }
    Log::trace("< setRouteAndLocationInfo");
    return (false);
}

std::string
Response::makeStatusLine()
{
    Log::trace("> makeStatusLine");
    std::string status_line;

    this->setStatusCode(std::string("400"));
    status_line = "HTTP/1.1 ";
    status_line += this->getStatusCode();
    status_line += " ";
    status_line += this->getStatusMessage(this->getStatusCode());
    status_line += "\r\n";
    Log::trace("< makeStatusLine");
    return (status_line);
}

// std::string
// Response::makeHeaders(Request& request)
// {
//     std::string headers;

//     headers += ft::getCurrentDateTime();
// }

// std::string
void
Response::makeBody(Request& request)
{
    Log::trace("> makeBody");
    (void)request;
    if (this->getResourceType() == ResType::AUTO_INDEX)
        PageGenerator::makeAutoIndex(*this);
    else if (this->getStatusCode().front() != '2')
        PageGenerator::makeErrorPage(*this);
    else // 일반적인 body
    {
    }
    Log::trace("< makeBody");
}

bool
Response::isLimitExceptInLocation()
{
    return (this->getLocationInfo().find("limit_except") != this->getLocationInfo().end());
}

bool
Response::isAllowedMethod(const std::string& method)
{
    return (this->_location_info["limit_except"].find(method) != std::string::npos);
}

void
Response::openCgiPipe()
{
    if (pipe(this->_cgi_pipe) == -1)
        throw (CannotOpenCgiPipeException(*this));
    close(this->_cgi_pipe[0]); // cgi_pipe[0] == cgi_pipe[SIDE_OUT] 이라고 생각하자.. 헷갈리니 메모해둠.
}

void
Response::appendBody(char *buf)
{
    this->setBody(this->getBody() + std::string(buf));
}