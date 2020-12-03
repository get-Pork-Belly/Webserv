#include "ServerGenerator.hpp"
#include "ServerManager.hpp"
#include "Server.hpp"
#include "Log.hpp"

/*============================================================================*/
/****************************  Static variables  ******************************/
/*============================================================================*/

/*============================================================================*/
/******************************  Constructor  *********************************/
/*============================================================================*/

ServerGenerator::ServerGenerator(ServerManager* server_manager)
: _server_manager(server_manager)
{
    this->convertFileToStringVector(this->_server_manager->getConfigFilePath());
}

/*============================================================================*/
/******************************  Destructor  **********************************/
/*============================================================================*/

ServerGenerator::~ServerGenerator()
{
}

/*============================================================================*/
/*******************************  Overload  ***********************************/
/*============================================================================*/

/*============================================================================*/
/********************************  Getter  ************************************/
/*============================================================================*/

/*============================================================================*/
/********************************  Setter  ************************************/
/*============================================================================*/

/*============================================================================*/
/******************************  Exception  ***********************************/

ServerGenerator::ConfigFileSyntaxError::ConfigFileSyntaxError(std::string msg)
: _msg(msg)
{
}

const char*
ServerGenerator::ConfigFileSyntaxError::what() const throw()
{
    return (this->_msg.c_str());
}

/*============================================================================*/
/*============================================================================*/
/*********************************  Util  *************************************/
/*============================================================================*/

void
ServerGenerator::convertFileToStringVector(const char *config_file_path)
{
    int	fd;
    int	readed;
    char buf[BUF_SIZE];
    std::string readed_string;
    std::vector<std::string> lines;

    fd = open(config_file_path, O_RDONLY, 0644);
    if (fd < 0)
        throw (strerror(errno));
    ft::memset(reinterpret_cast<void *>(buf), 0, BUF_SIZE);
    while ((readed = read(fd, reinterpret_cast<void *>(buf), BUF_SIZE)))
    {
        if (readed < 0)
            throw(strerror(errno));
        readed_string += std::string(buf);
    }
    lines = ft::split(readed_string, "\n");
    for (std::string line : lines)
    {
        std::string trimmed = ft::ltrim(ft::rtrim(line));
        if (trimmed.size() > 0)
            this->_configfile_lines.push_back(trimmed);
    }
}

void
ServerGenerator::setDefaultRouteOfServer(std::map<std::string, location_info>& locations,
                            server_info& server_config)
{
    location_info info;

    if (locations.size() == 0)
    {
        this->initLocationConfig(info, server_config);
        locations["/"] = info;
    }
    else
    {
        if (locations.find("/") != locations.end())
            return ;
        this->initLocationConfig(info, server_config);
        locations["/"] = info;
    }
}

void 
ServerGenerator::generateServers(std::vector<Server *>& servers)
{
    server_info http_config;
    std::vector<std::string> directives;
    
    std::vector<std::string>::iterator it = this->_configfile_lines.begin();
    std::vector<std::string>::iterator ite = this->_configfile_lines.end();
    http_config = this->parseHttpBlock();

    while (it != ite)
    {
        if ( *it++ == "server {")
        {
            server_info server_config;
            this->initServerConfig(server_config, http_config);
            std::map<std::string, location_info> locations;
            this->parseServerBlock(it, server_config, locations);
            this->setDefaultRouteOfServer(locations, server_config);
            testServerConfig(server_config);
            testLocationConfig(locations);
            servers.push_back(new Server(this->_server_manager, server_config, locations));
        }
    }
}

void
ServerGenerator::checkHttpBlock(std::vector<std::string>::iterator& it, const std::vector<std::string>::iterator& ite)
{
    while (it != ite)
    {
        if (*it == "http {")
            break ;
        it++;
    }
    if (it == ite)
        throw (ConfigFileSyntaxError(NO_HTTP_BLOCK));
}

bool
ServerGenerator::isEmptyLine(const std::string& line)
{
    for (char c : line)
    {
        if (c == ' ' || c == '\t')
            continue ;
        return (false);
    }
    return (true);
}

server_info
ServerGenerator::parseHttpBlock()
{
    server_info http_config;
    server_info skip_server;
    std::map<std::string, location_info> skip_locations;
    std::vector<std::string>::iterator it = this->_configfile_lines.begin();
    std::vector<std::string>::iterator ite = this->_configfile_lines.end();

    this->initHttpConfig(http_config);
    checkHttpBlock(it, ite);

    while (it++ != ite)
    {
        if (it == ite)
            throw (ConfigFileSyntaxError(BRACKET_ERROR));
        if (this->isEmptyLine(*it))
            continue ;
        if (*it == "server {")
            (*this.*skipServerBlock)(it, skip_server, skip_locations);
        if (*it == "}")
            break ;
        else if (*it != "}")
        {
            if ((*it).back() == ';')
            {
                size_t pos = (*it).find(" ");
                std::string key = (*it).substr(0, pos);
                std::string value = ft::ltrim(ft::rtrim((*it).substr(pos), "; "), " ");
                http_config[key] = value;
            }
            else
                throw (ConfigFileSyntaxError(NO_SEMICOLON));
        }
    }
    return (http_config);
}

void
ServerGenerator::parseServerBlock(std::vector<std::string>::iterator& it, server_info& server_config, std::map<std::string, location_info>& locations)
{
    std::vector<std::string>	directives;

    while (it != this->_configfile_lines.end())
    {
        if (*it == "server {" || this->isEmptyLine(*it))
        {
            it++;
            continue ;
        }
        directives = ft::split(ft::ltrim(ft::rtrim(*it, " \t"), " \t"), " ");
        if (directives.size() == 1 && directives[0] == "}")
            return ;
        else if (directives[0] == "location")
        {
            location_info location_config = this->parseLocationBlock(it, server_config);
            locations[location_config["route"]] = location_config;
            continue ;
        }
        else
        {
            setDirectiveToConfig(server_config, directives);
            it++;
        }
    }
    if (it == this->_configfile_lines.end())
        throw (ConfigFileSyntaxError(BRACKET_ERROR));
}

void
ServerGenerator::setDirectiveToConfig(std::map<std::string, std::string>& config, std::vector<std::string>& directive)
{
    std::string joined;

    if (directive[directive.size() - 1].back() != ';')
        throw (ConfigFileSyntaxError(NO_SEMICOLON));
    else
        directive[directive.size() - 1].pop_back();
    if (directive.size() == 1)
        throw (ConfigFileSyntaxError(NO_ARGUMENT));
    if (directive.size() > 2)
    {
        for (size_t i = 1; i < directive.size(); ++i)
        {
            joined += directive[i];
            joined += " ";
        }
        config[directive[0]] = joined;
    }
    else
        config[directive[0]] = directive[1];
}

location_info
ServerGenerator::parseLocationBlock(std::vector<std::string>::iterator& it, server_info& server_config)
{
    std::vector<std::string> directives;
    location_info location_config;

    this->initLocationConfig(location_config, server_config);
    directives = ft::split(ft::ltrim(ft::rtrim(*it++, " \t"), " \t"), " ");
    if (directives.size() != 3 || directives[2] != "{")
        throw (ConfigFileSyntaxError(INVALID_BLOCK));
    location_config["route"] = directives[1];
    while (it != this->_configfile_lines.end())
    {
        directives = ft::split(ft::ltrim(ft::rtrim(*it++, " \t"), " \t"), " ");
        if (directives.size() == 0)
            continue ;
        else if (directives[0] == "}")
            return (location_config);
        else if (directives[0] == "location")
            throw (ConfigFileSyntaxError(BRACKET_ERROR));
        else
            setDirectiveToConfig(location_config, directives);
    }
    if (it == this->_configfile_lines.end())
        throw (ConfigFileSyntaxError(BRACKET_ERROR));
    return (location_config);
}

void
ServerGenerator::initHttpConfig(server_info& http_config)
{
    http_config["root"] = "html";
    http_config["index"] = "index.html";
    http_config["autoindex"] = "off";
    http_config["auth_basic"] = "off";
    http_config["retry_after_sec"] = "180";
}

void
ServerGenerator::initServerConfig(server_info& server_config, server_info& http_config)
{
    const std::map<std::string, std::string>::iterator& ite = http_config.end();

    server_config["listen"] = std::to_string(8080);
    server_config["client_max_body_size"] = std::string("1m");
    if (http_config.find("root") != ite)
        server_config["root"] = http_config["root"];
    if (http_config.find("index") != ite)
        server_config["index"] = http_config["index"];
    if (http_config.find("autoindex") != ite)
        server_config["autoindex"] = http_config["autoindex"];
    if (http_config.find("auth_basic") != ite)
        server_config["auth_basic"] = http_config["auth_basic"];
    if (http_config.find("auth_basic_user_file") != ite)
        server_config["auth_basic_user_file"] = http_config["auth_basic_file"];
    if (http_config.find("error_page") != ite)
        server_config["error_page"] = http_config["error_page"];
    if (http_config.find("retry_after_sec") != ite)
        server_config["retry_after_sec"] = http_config["retry_after_sec"];
}

void
ServerGenerator::initLocationConfig(location_info& location_config, server_info& server_config)
{
    const std::map<std::string, std::string>::iterator& ite = server_config.end();

    if (server_config.find("root") != ite)
        location_config["root"] = server_config["root"];
    if (server_config.find("index") != ite)
        location_config["index"] = server_config["index"];
    if (server_config.find("autoindex") != ite)
        location_config["autoindex"] = server_config["autoindex"];
    if (server_config.find("auth_basic") != ite)
        location_config["auth_basic"] = server_config["auth_basic"];
    if (server_config.find("auth_basic_user_file") != ite)
        location_config["auth_basic_user_file"] = server_config["auth_basic_file"];
    if (server_config.find("error_page") != ite)
        location_config["error_page"] = server_config["error_page"];
    if (server_config.find("retry_after_sec") != ite)
        location_config["retry_after_sec"] = server_config["retry_after_sec"];
}

/* 디버깅용 함수 */
void testServerConfig(server_info& test)
{
    std::cout << "\033[1;31;40mserver config check\033[0m" << std::endl;
    for (auto& s : test)
        std::cout << "key:" << "\033[1;31;40m" << std::setw(25) << s.first << "\033[0m" << "||  value:" << "\033[1;34;40m" << s.second << "\033[0m" << std::endl;
    std::cout << "=====================================" << std::endl;
}

void testLocationConfig(std::map<std::string, location_info>& test)
{
    std::cout << "\033[1;31;40mlocation config check\033[0m" << std::endl;
    for (auto& a : test)
    {
        std::cout << "route: " << a.first << std::endl;
        location_info temp = a.second;
        for (auto& b : temp)
        {
            std::cout << "key: "<< "\033[1;31;40m" << std::setw(25) << b.first << "\033[0m" <<  "||  value: " << "\033[1;34;40m" << b.second << "\033[0m" << std::endl;
        }
        std::cout << "=====================================" << std::endl;
    }
}
