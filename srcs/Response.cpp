#include "Response.hpp"
#include "Server.hpp"

/*============================================================================*/
/****************************  Static variables  ******************************/
/*============================================================================*/

/*============================================================================*/
/******************************  Constructor  *********************************/
/*============================================================================*/

Response::Response()
: _status_code(""), _status_description(""), _transfer_type(""),
_clients(""), _message_body("") 
{
    this->_headers = { {"", ""} };
    this->initStatusCodeTable();
}

Response::Response(const Response& other)
: _status_code(other._status_code), _status_description(other._status_description), 
_headers(other._headers), _transfer_type(other._transfer_type),
_clients(other._clients), _message_body(other._message_body),
_status_code_table(other._status_code_table) {}

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
    this->_status_description = rhs._status_description;
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
Response::setStatusCode(std::string& status_code)
{
    this->_status_code = status_code;
}

void
Response::setStatusDescription()
{
    this->_status_description = this->_status_code_table[this->getStatusCode()];
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
    this->_status_description = "";
    this->_headers = { {"", ""} };
    this->_transfer_type = "";
    this->_clients = "";
    this->_message_body = "";
}

//TODO Not yet completed
void 
Response::initAndUpdate(Request& request)
{
    this->init();
    this->setStatusCode(request);
    if (this->getStatusCode()[0] != '4')
    {
    }
    this->setStatusDescription();
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
