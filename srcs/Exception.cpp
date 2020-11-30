#include "Exception.hpp"

SendErrorCodeToClientException::SendErrorCodeToClientException()
{
}

const char*
SendErrorCodeToClientException::what() const throw()
{
    return ("[SendErrorCodeToClientException] <-- overloaded");
}

CannotSendErrorCodeToClientException::CannotSendErrorCodeToClientException()
{
}

const char*
CannotSendErrorCodeToClientException::what() const throw()
{
    return ("[CannotSendErrorCodeToClientException] <-- overloaded");
}
