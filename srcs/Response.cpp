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

/*============================================================================*/
/********************************  Setter  ************************************/
/*============================================================================*/

void
Response::setStatusCode(Request& request)
{
    this->_status_code = request.getStatusCode();
}

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
Response::getStatusMessage(const std::string& code)
{
    return (this->_status_code_table[code]);
}

std::string
Response::makeStartLine()
{
    std::string start_line;

    this->setStatusCode(std::string("400"));
    start_line = "HTTP/1.1 ";
    start_line += this->getStatusCode();
    start_line += " ";
    start_line += this->getStatusMessage(this->getStatusCode());
    start_line += "\r\n";
    return (start_line);
}
