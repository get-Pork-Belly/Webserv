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

const std::vector<std::string>&
UriParser::getPaths() const
{
    return (this->_paths);
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
UriParser::setPaths()
{
    size_t i = 0;
    if (this->_path == "/")
    {
        this->_paths.push_back("/");
        return ;
    }
    this->_paths = ft::split(this->_path, "/");
    for (; i < this->_paths.size() - 1; i++)
        this->_paths[i].append("/");
    if (this->_uri.back() == '/')
        this->_paths[i].append("/");
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

void
UriParser::parseUri(const std::string& uri)
{
    this->setUri(uri);
    this->setScheme(this->findScheme());
    const std::string& host_port = this->findHostAndPort();
    size_t index = this->getIndex();
    this->setHostAndPort(host_port);
    if (index == std::string::npos || this->_uri.size() == index)
    {
        this->setPath("/");
        this->setPaths();
    }
    else
    {
        this->setPath(this->findPath());
        this->setPaths();
    }
    this->findAndSetQuery(this->getPath());
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
        // this->setIndex(this->_index + 1); // NOTE
        this->setIndex(this->_index); // NOTE
        return ("");
    }
    found = this->_uri.find("/", this->_index);
    if (found == std::string::npos)
    {
        this->setIndex(found);
        return (this->_uri.substr(temp_index));
    }
    // this->setIndex(found + 1); // NOTE
    this->setIndex(found); // NOTE
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
    this->_paths.clear();
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
