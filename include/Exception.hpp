#ifndef EXCEPTION_HPP
# define EXCEPTION_HPP

#include <iostream>

class SendErrorCodeToClientException : public std::exception
{
public:
    SendErrorCodeToClientException();
    virtual const char* what() const throw();
};

#endif
