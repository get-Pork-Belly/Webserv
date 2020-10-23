#include "Response.hpp"
#include "Server.hpp"

/*============================================================================*/
/****************************  Static variables  ******************************/
/*============================================================================*/

/*============================================================================*/
/******************************  Constructor  *********************************/
/*============================================================================*/

Response::Response()
: _status_code(""), _transfer_type(""), _clients(""), _message_body("") 
{
    this->_headers = { {"", ""} };
    this->initStatusCodeTable();
}

Response::Response(const Response& other)
: _status_code(other._status_code),  _headers(other._headers),
_transfer_type(other._transfer_type), _clients(other._clients),
_message_body(other._message_body), _status_code_table(other._status_code_table) {}

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
    this->_message_body = rhs._message_body;
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

const std::string&
Response::getLocation() const
{
    return (this->_location);
}

/*============================================================================*/
/********************************  Setter  ************************************/
/*============================================================================*/

void
Response::setStatusCode(const std::string& status_code)
{
    this->_status_code = status_code;
}

/*============================================================================*/
/******************************  Exception  ***********************************/
/*============================================================================*/

/*============================================================================*/
/*********************************  Util  *************************************/
/*============================================================================*/

void
Response::init()
{
    this->_status_code = "";
    this->_headers = { {"", ""} };
    this->_transfer_type = "";
    this->_clients = "";
    this->_message_body = "";
}

void
Response::initStatusCodeTable()
{
    this->_status_code_table = {
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
        {"417", "Expectation Failed"}
    };
}

std::string
Response::makeStatusLine()
{
    std::string status_line;

    this->setStatusCode(std::string("400"));
    status_line = "HTTP/1.1 ";
    status_line += this->getStatusCode();
    status_line += " ";
    status_line += this->getStatusMessage(this->getStatusCode());
    status_line += "\r\n";
    return (status_line);
}

void
Response::applyAndCheckRequest(Request& request, Server* server)
{
    this->setStatusCode(request.getStatusCode());
    if (checkAndSetLocation(request.getRequestUri(), server))
    {
        std::cout << "True! " << std::endl;
        if (isLimitExceptInLocation(server) == true)
            std::cout<<"Yes limit exist"<<std::endl;
        else
            std::cout<<"Yes limit exist"<<std::endl;
        // if (isLimitExceptInLocation() == true && !isAllowedMethod())
        //     this->setStatusCode("405");
    }
    std::cout << "False" << std::endl;
}

bool
Response::checkAndSetLocation(const std::string& uri, Server* server)
{
    std::map<std::string, location_info> location_config = server->getLocationConfig();
    std::string router;

    if (uri[0] != '/')
        return (false);
    if (uri.length() == 1)
    {
        if (location_config.find("/") != location_config.end())
        {
            this->_location = "/";
            return (true);
        }
        return (false);
    }
    size_t index = uri[uri.length() - 1] == '/' ? uri.length() : uri.length() + 1;
    while ((index = uri.find_last_of("/", index - 1)) != std::string::npos)
    {
        router = uri.substr(0, index);
        if (location_config.find(router) != location_config.end())
        {
            this->_location = router;
            return (true);
        }
        if (index == 0)
            break ;
    }
    return (false);
}

bool
Response::isLimitExceptInLocation(Server* server)
{
    std::map<std::string, location_info> location_config = server->getLocationConfig();
    return (location_config[this->getLocation()].find("limit_except") != location_config[this->getLocation()].end());
}

// bool
// Response::isAllowedMethod()
// {

// }