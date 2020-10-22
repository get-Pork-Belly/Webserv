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

// void 
// Response::initAndUpdate(Request& request)
// {
//     this->init();
//     this->setStatusCode(request);
//     if (this->getStatusCode()[0] != '4')
//     {
//         // 헤더를 읽어서 필요한 응답 생성 하며 상태코드 셋팅
//         // ex) CGI 실행, html body 불러오기
//     }
//     this->setStatusDescription();
// }

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
    if (isLocationUri(request.getRequestUri(), server))
    {
        std::cout<<"It's location!"<<std::endl;
        // if (isExistLocation() == true && isExistLimitExcept() == true)
        // {
        //     if (!isAllowedMethod())
        //         this->setStatusCode("405");
        // }
    }
    else
        std::cout<<"It's not location!"<<std::endl;
}

bool
Response::isLocationUri(const std::string& uri, Server* server)
{
    if (uri[0] != '/')
        return (false);

    size_t index = uri[uri.length() - 1] == '/' ? uri.length() : uri.length() + 1;
    std::map<std::string, location_info> location_config = server->getLocationConfig();
    std::string router;
    while ((index = uri.find_last_of("/", index - 1)) != std::string::npos)
    {
        router = uri.substr(0, index);
        if (location_config.find(router) != location_config.end())
            return (true);
        if (index == 0)
            break ;
    }
    return (false);
}

// bool
// Response::isExistLocation()
// {

// }

// bool
// Response::isExistLimitExcept()
// {

// }

// bool
// Response::isAllowedMethod()
// {

// }