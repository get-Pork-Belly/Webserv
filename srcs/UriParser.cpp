#include "UriParser.hpp"

/*============================================================================*/
/****************************  Static variables  ******************************/
/*============================================================================*/

/*============================================================================*/
/******************************  Constructor  *********************************/
/*============================================================================*/

UriParser::UriParser()
{
    this->init();
}

/*============================================================================*/
/******************************  Destructor  **********************************/
/*============================================================================*/

UriParser::~UriParser()
{ }

/*============================================================================*/
/*******************************  Overload  ***********************************/
/*============================================================================*/

/*============================================================================*/
/********************************  Getter  ************************************/
/*============================================================================*/

int
UriParser::getIndex() const
{
    return (this->_index);
}

const std::string&
UriParser::getScheme() const
{
    return (this->_scheme);
}

const std::string&
UriParser::getHost() const
{
    return (this->_host);
}

const std::string&
UriParser::getPort() const
{
    return (this->_port);
}

const std::string&
UriParser::getPath() const
{
    return (this->_path);
}

const std::string&
UriParser::getQuery() const
{
    return (this->_query);
}

/*============================================================================*/
/********************************  Setter  ************************************/
/*============================================================================*/

void
UriParser::setIndex(int index)
{
    this->_index = index;
}

void
UriParser::setUri(const std::string& uri)
{
    this->_uri = uri;
}

void
UriParser::setScheme(const std::string& scheme)
{
    this->_scheme = scheme;
}

void
UriParser::setHost(const std::string& host)
{
    this->_host= host;
}

void
UriParser::setPort(const std::string& port)
{
    this->_port = port;
}

void
UriParser::setPath(const std::string& path)
{
    this->_path = path;
}

void
UriParser::setHostAndPort(const std::string& host_port)
{
    size_t found = host_port.find(":");
    if (found == std::string::npos)
    {
        this->setHost(host_port);
        this->setPort("");
        return ;
    }
    std::vector<std::string> splited = ft::split(host_port, ":");
    this->setHost(splited[0]);
    this->setPort(splited[1]);
}

void
UriParser::setQuery(const std::string& query)
{
    this->_query = query;
}

/*============================================================================*/
/******************************  Exception  ***********************************/
/*============================================================================*/

/*============================================================================*/
/*********************************  Util  *************************************/
/*============================================================================*/

bool
UriParser::decodingUri(std::string& uri)
{
    size_t target_index = 0;
    while ((target_index = uri.find('+', target_index)) != std::string::npos)
        uri.replace(target_index, 1, " ");
    target_index = 0;
    while ((target_index = uri.find('%', target_index)) != std::string::npos)
    {
        std::string hex = uri.substr(target_index + 1, 2);
        int base10 = ft::stoiHex(hex);
        if (base10 < 31 || base10 >= 127)
            return (false);
        uri.replace(target_index, 3, 1, static_cast<unsigned char>(base10));
        target_index += 1;
    }
    return (true);
}

bool
UriParser::parseUri(const std::string& uri)
{
    std::string parsed_uri(uri);
    if (this->decodingUri(parsed_uri) == false)
        return (false);
    this->setUri(parsed_uri);
    this->setScheme(this->findScheme());
    const std::string& host_port = this->findHostAndPort();
    size_t index = this->getIndex();
    this->setHostAndPort(host_port);
    if (index == std::string::npos || this->_uri.size() == index)
        this->setPath("/");
    else
        this->setPath(this->findPath());
    this->findAndSetQuery(this->getPath());
    return (true);
}

std::string
UriParser::findScheme()
{
    std::string scheme;
    size_t temp_index = this->getIndex();

    size_t found = this->_uri.find("://", this->_index);
    if (found == std::string::npos)
        return ("");
    this->setIndex(found + 3);
    return (this->_uri.substr(temp_index, found));
}

std::string
UriParser::findHostAndPort()
{
    size_t found;
    std::string host_port;
    size_t temp_index = this->getIndex();

    if (this->_uri[this->_index] == '/')
    {
        this->setIndex(this->_index);
        return ("");
    }
    found = this->_uri.find("/", this->_index);
    if (found == std::string::npos)
    {
        this->setIndex(found);
        return (this->_uri.substr(temp_index));
    }
    this->setIndex(found);
    return (this->_uri.substr(temp_index, found - temp_index));
}

std::string
UriParser::findPath()
{
    if (this->_uri.length() == 1 && this->_uri[0] == '/')
        return ("/");
    else
        return (this->_uri.substr(this->_index, this->_uri.length() - this->_index));
}

void
UriParser::findAndSetQuery(const std::string& path)
{
    size_t index = path.find("?");
    if (index == std::string::npos)
        this->setQuery("");
    else
    {
        if (path.length() > index + 1)
            this->setQuery(path.substr(index + 1));
        else
            this->setQuery("");
        this->setPath(path.substr(0, index));
    }
}

void
UriParser::init()
{
    this->setIndex(0);
    this->_uri.clear();
    this->_scheme.clear();
    this->_host.clear();
    this->_port.clear();
    this->_path.clear();
    this->_query.clear();
}

void
UriParser::print()
{
    std::cout << "scheme: " << this->getScheme() << std::endl;
    std::cout << "host: " << this->getHost() << std::endl;
    std::cout << "port: " << this->getPort() << std::endl;
    std::cout << "path: " << this->getPath() << std::endl;
    std::cout << "query: " << this->getQuery() << std::endl;
}
