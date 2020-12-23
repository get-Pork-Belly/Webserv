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
    if (!this->isConfigFileBracketAllPaired())
        throw (ConfigFileSyntaxError(BRACKET_ERROR));
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
ServerGenerator::checkValidationOfConfigs(server_info& server, std::map<std::string, location_info>& locations)
{
    this->checkValidationOfServerConfig(server);
    this->checkValidationOfLocationConfig(locations);
}

void
ServerGenerator::checkValidationOfLocationConfig(std::map<std::string, location_info>& locations)
{
    std::vector<std::string> list =
    {"autoindex", "auth_basic", "auth_basic_user_file", "cgi", "cgi_path",
     "index", "return","retry_after_sec", "route", "root", "limit_except",
     "limit_client_body_size", "plugin", "timeout_second", "log_at", "echo"};
    for (auto& location : locations)
    {
        for (auto info : location.second)
        {
            std::vector<std::string>::iterator it = std::find(list.begin(), list.end(), info.first);
            std::vector<std::string> value;
            if (it == list.end())
            {
                std::cerr << info.first << " ";
                throw (ConfigFileSyntaxError(INVALID_DIRECtiVE));
            }
            if (info.first == "return")
            {
                value = ft::split(info.second, " ");
                if (value.size() != 2)
                    throw (ConfigFileSyntaxError(INVALID_VALUE));
                if (value[0].length() != 3 || value[0].front() != '3')
                    throw (ConfigFileSyntaxError(INVALID_VALUE));
            }
            else if (info.first == "cgi")
            {
                value = ft::split(info.second, " ");
                for (auto& arg : value)
                {
                    if (arg.front() != '.')
                        throw (ConfigFileSyntaxError(INVALID_VALUE));
                }
            }
        }
    }
}

void
ServerGenerator::checkValidationOfServerConfig(server_info& server)
{
    std::vector<std::string> list =
    {"autoindex", "auth_basic", "auth_basic_user_file", "index",
     "retry_after_sec", "route", "root", "limit_except", "listen",
     "limit_client_body_size", "server_name", "plugin", "timeout_second",
     "log_at"};
    for (auto& directive : server)
    {
        std::vector<std::string> value;
        std::vector<std::string>::iterator it = std::find(list.begin(), list.end(), directive.first);
        if (it == list.end())
        {
            std::cerr << directive.first << " ";
            throw (ConfigFileSyntaxError(INVALID_DIRECtiVE));
        }
    }
}

//TODO: server generate 중 에러 발생시 catch 해줄 것.
void 
ServerGenerator::generateServers(std::vector<Server *>& servers)
{

    try
    {

        server_info http_config;
        bool server_block_exists = false;
        
        std::vector<std::string>::iterator it = this->_configfile_lines.begin();
        std::vector<std::string>::iterator ite = this->_configfile_lines.end();
        http_config = this->parseHttpBlock();
        this->_server_manager->setPlugins(http_config);

        while (it != ite)
        {
            if ( *it++ == "server {")
            {
                server_info server_config;
                this->initServerConfig(server_config, http_config);
                std::map<std::string, location_info> locations;
                this->parseServerBlock(it, server_config, locations);
                this->checkValidationOfConfigs(server_config, locations);
                this->setDefaultRouteOfServer(locations, server_config);
                server_block_exists = true;

                testServerConfig(server_config);
                testLocationConfig(locations);
                Server* server = new Server(this->_server_manager, server_config, locations);
                servers.push_back(server);
            }
        }
        if (server_block_exists == false)
            throw ("There is no server block in config_file!");
    }
    catch (std::bad_alloc& e)
    {
        for (auto& s : servers)
            delete (s);
        throw ("new Failed while generating server");
    }
    catch (std::exception& e)
    {
        std::cerr << "ServerGenerateError: " << e.what() << std::endl;
        throw (e);
    }
}

bool
ServerGenerator::httpBlockExists(std::vector<std::string>::iterator& it, const std::vector<std::string>::iterator& ite)
{
    while (it != ite)
    {
        if (*it == "http {")
            return (true);
        it++;
    }
    return (false);
}

bool
ServerGenerator::isConfigFileBracketAllPaired() const
{
    //NOTE: line by line, { 이나 }가 한줄에 하나만 있어야한다. 이에 따르지 않으면 undefined behavior가 발생한다.
    //NOTE: 또한 파일명, 폴더명에 '{' or '}' 이 포함되어있다면 undefined behavior가 발생한다.
    int count_not_paired_bracket = 0;
    for (auto& line : this->_configfile_lines)
    {
        if (line.find("{") != std::string::npos)
            count_not_paired_bracket++;
        if (line.find("}") != std::string::npos)
            count_not_paired_bracket--;
    }
    if (count_not_paired_bracket == 0)
        return (true);
    return (false);
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
    if (!httpBlockExists(it, ite))
        throw (ConfigFileSyntaxError(NO_HTTP_BLOCK));

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
                if (pos == std::string::npos)
                    throw (ConfigFileSyntaxError(INVALID_DIRECtiVE));
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
    std::string value;

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
            value += directive[i];
            value += " ";
        }
        config[directive[0]] = value;
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
    http_config["root"] = "/Users/iwoo/Documents/Webserv";
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
    if (http_config.find("plugin") != ite)
        server_config["plugin"] = http_config["plugin"];
    if (http_config.find("timeout_second") != ite)
        server_config["timeout_second"] = http_config["timeout_second"];
    if (http_config.find("log_at") != ite)
        server_config["log_at"] = http_config["log_at"];    
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
    if (server_config.find("plugin") != ite)
        location_config["plugin"] = server_config["plugin"];
    if (server_config.find("timeout_second") != ite)
        location_config["timeout_second"] = server_config["timeout_second"];
    if (server_config.find("log_at") != ite)
        location_config["log_at"] = server_config["log_at"];    
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
