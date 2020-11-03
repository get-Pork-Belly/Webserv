#include "Exception.hpp"

SendErrorCodeToClientException::SendErrorCodeToClientException()
{
}

const char*
SendErrorCodeToClientException::what() const throw()
{
    return ("[SendErrorCodeToClientException] <-- overloaded");
}
