#include "UrlParser.hpp"

/*============================================================================*/
/****************************  Static variables  ******************************/
/*============================================================================*/

/*============================================================================*/
/******************************  Constructor  *********************************/
/*============================================================================*/

UrlParser::UrlParser()
:_index(0), _url(""), _scheme(""), _host(""), _port(""), _path("") {}

/*============================================================================*/
/******************************  Destructor  **********************************/
/*============================================================================*/

UrlParser::~UrlParser()
{ }

/*============================================================================*/
/*******************************  Overload  ***********************************/
/*============================================================================*/

/*============================================================================*/
/********************************  Getter  ************************************/
/*============================================================================*/

int
UrlParser::getIndex() const
{
    return (this->_index);
}

const std::string&
UrlParser::getScheme() const
{
    return (this->_scheme);
}

const std::string&
UrlParser::getHost() const
{
    return (this->_host);
}

const std::string&
UrlParser::getPort() const
{
    return (this->_port);
}
/*============================================================================*/
/********************************  Setter  ************************************/
/*============================================================================*/

void
UrlParser::setIndex(int index)
{
    this->_index = index;
}

void
UrlParser::setUrl(const std::string& url)
{
    this->_url = url;
}

void
UrlParser::setScheme(const std::string& scheme)
{
    this->_scheme = scheme;
}

void
UrlParser::setHost(const std::string& host)
{
    this->_host= host;
}

void
UrlParser::setPort(const std::string& port)
{
    this->_port = port;
}

void
UrlParser::setPath(const std::string& path)
{
    this->_path = path;
}

void
UrlParser::setHostAndPort(const std::string& host_port)
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
UrlParser::setPaths()
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
    if (this->_url.back() == '/')
        this->_paths[i].append("/");
}

/*============================================================================*/
/******************************  Exception  ***********************************/
/*============================================================================*/

/*============================================================================*/
/*********************************  Util  *************************************/
/*============================================================================*/

void
UrlParser::parseUrl(const std::string& url)
{
    this->setUrl(url);
    this->setScheme(this->findScheme());
    const std::string& host_port = this->findHostAndPort();
    size_t index = this->getIndex();
    this->setHostAndPort(host_port);
    if (index == std::string::npos || this->_url.size() == index)
    {
        this->setPath("/");
        this->setPaths();
        this->print();
    }
    else
    {
        this->setPath(this->findPath());
        this->setPaths();
        this->print();
    }
}

std::string
UrlParser::findScheme()
{
    std::string scheme;
    size_t temp_index = this->getIndex();

    size_t found = this->_url.find("://", this->_index);
    if (found == std::string::npos)
        return ("");
    this->setIndex(found + 3);
    return (this->_url.substr(temp_index, found));
}

std::string
UrlParser::findHostAndPort()
{
    size_t found;
    std::string host_port;
    size_t temp_index = this->getIndex();

    if (this->_url[this->_index] == '/')
    {
        this->setIndex(this->_index + 1);
        return ("");
    }
    found = this->_url.find("/", this->_index);
    if (found == std::string::npos)
    {
        this->setIndex(found);
        return (this->_url.substr(temp_index));
    }
    this->setIndex(found + 1);
    return (this->_url.substr(temp_index, found - temp_index));
}

std::string
UrlParser::findPath()
{
    if (this->_url.length() == 1 && this->_url[0] == '/')
        return ("/");
    else
        return (this->_url.substr(this->_index, this->_url.length() - this->_index));
}

void
UrlParser::clear()
{
    this->setIndex(0);
    this->_url.clear();
    this->_scheme.clear();
    this->_host.clear();
    this->_port.clear();
    this->_path.clear();
    this->_paths.clear();
}

void
UrlParser::print()
{
    std::cout << "scheme: " << this->_scheme << std::endl;
    std::cout << "host: " << this->_host << std::endl;
    std::cout << "port: " << this->_port << std::endl;
    std::cout << "path: " << this->_path << std::endl;
    for (auto& s : this->_paths)
        std::cout << "paths: " << s << std::endl;
}
