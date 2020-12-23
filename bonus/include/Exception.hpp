#ifndef EXCEPTION_HPP
# define EXCEPTION_HPP

#include <iostream>

class SendErrorCodeToClientException : public std::exception
{
public:
    SendErrorCodeToClientException();
    virtual const char* what() const throw();
};

class CannotSendErrorCodeToClientException : public std::exception
{
public:
    CannotSendErrorCodeToClientException();
    virtual const char* what() const throw();
};

class ConfigFileErrorException : public std::exception
{
public:
    ConfigFileErrorException();
    virtual const char* what() const throw();
};

#endif
