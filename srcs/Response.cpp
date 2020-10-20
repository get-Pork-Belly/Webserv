#include "Response.hpp"

/*============================================================================*/
/****************************  Static variables  ******************************/
/*============================================================================*/

/*============================================================================*/
/******************************  Constructor  *********************************/
/*============================================================================*/

Response::Response()
: _status_code(""), _status_description(""), _headers(""), _transfer_type(""),
_clients(""), _message_body("") {}

Response::Response(const Response& other)
: _status_code(other._status_code), _status_description(other._status_description), 
_headers(other._headers), _transfer_type(other._transfer_type),
_clients(other._clients), _message_body(other._message_body) {}

Response::Response(Request& req, Server* server)
{
    (void)server;
    this->setStatusCode(req);
    if (getStatusCode()[0] == '4')
    {
    }
}

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

/*============================================================================*/
/******************************  Exception  ***********************************/
/*============================================================================*/

/*============================================================================*/
/*********************************  Util  *************************************/
/*============================================================================*/
