#include "ServerGenerator.hpp"
#include "ServerManager.hpp"

/*============================================================================*/
/****************************  Static variables  ******************************/
/*============================================================================*/

/*============================================================================*/
/******************************  Constructor  *********************************/
/*============================================================================*/

ServerGenerator::ServerGenerator(const ServerManager* server_manager)
: _server_manager(server_manager)
{
    this->convertFileToStringVector(this->_server_manager->getConfigFilePath());
    // TODO 구현하기
    // if (!this->isValidConfigFile())
    //     throw "config file error"; // throw
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
    memset(reinterpret_cast<void *>(buf), 0, BUF_SIZE);
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
ServerGenerator::generateServers(std::vector<Server *>& servers)
{
    type_server http_config;
    std::vector<std::string> directives;
    
    std::vector<std::string>::iterator it = this->_configfile_lines.begin();
    std::vector<std::string>::iterator ite = this->_configfile_lines.end();
    http_config = parseHttpBlock();
    (void)servers;

    while (it != ite)
    {
        if ( *it == "server {")
        {
            type_server server_config(http_config);
            it++;
            std::map<std::string, type_location> locations;
            parseServerBlock(it, server_config, locations);
            // testLocation(locations);
            testServer(server_config);

            // servers.push_back(new Server(server_config, locations));
        }
        it++;
    }
}

type_server
ServerGenerator::parseHttpBlock()
{
    type_server http_config;
    std::vector<std::string>::iterator it = this->_configfile_lines.begin();
    std::vector<std::string>::iterator ite = this->_configfile_lines.end();

    while (it != ite)
    {
        if (*it == "http {")
            break ;
        it++;
    }
    while (it != ite)
    {
        if ((*it).back() == ';')
        {
            size_t pos = (*it).find(" ");
            std::string key = (*it).substr(0, pos);
            std::string value = ft::ltrim(ft::rtrim((*it).substr(pos), "; "), " ");
            http_config[key] = value;
        }
        else if (*it == "server {")
        {
            type_server server_temp;
            std::map<std::string, type_location> locations_temp;
            parseServerBlock(it, server_temp, locations_temp);
        }
        it++;
    }
    return (http_config);
}

void
ServerGenerator::parseServerBlock(std::vector<std::string>::iterator& it, type_server& server_config, std::map<std::string, type_location>& locations)
{
    std::vector<std::string>	directives;

    while (it != _configfile_lines.end())
    {
        directives = ft::split(*it, " ");
        if (directives[0] == "location")
        {
            type_location location_config = parseLocationBlock(it, server_config);
            std::string temp = location_config["route"];
            locations[temp] = location_config;
            continue ;
        }
        if (directives[0] == "}")
            return ;
        std::string temp = ft::rtrim(directives[1], ";");
        directives[1] = temp;
        server_config[directives[0]] = directives[1];
        directives.clear();
        it++;
    }
}

type_location
ServerGenerator::parseLocationBlock(std::vector<std::string>::iterator& it, type_server& server_config)
{
    std::vector<std::string> directives;
    type_location location_config;

    setLocationConfig(location_config, server_config);
    while (it != _configfile_lines.end())
    {
        directives = ft::split(*it, " ");
        if (directives[0] == "location")
        {
            location_config["route"] = directives[1];
            it++;
            continue ;
        }
        if (directives[0] == "}")
        {
            it++;
            return (location_config);
        }
        std::string joined;
        if (directives.size() > 2)
        {
            for (size_t i = 1; i < directives.size(); ++i)
            {
                joined += directives[i];
                joined += " ";
            }
            joined = ft::rtrim(joined, "; ");
            location_config[directives[0]] = joined;
        }
        else
        {
            std::string temp = ft::rtrim(directives[1], ";");
            location_config[directives[0]] = temp;
        }
        directives.clear();
        it++;
    }
    return (location_config);
}

void
ServerGenerator::setLocationConfig(type_location& location_config, type_server& server_config)
{
    std::map<std::string, std::string>::iterator ite = server_config.end();

    if (server_config.find("index") != ite)
        location_config["index"] = server_config["index"];
    if (server_config.find("root") != ite)
        location_config["root"] = server_config["root"];
}

/* 디버깅용 함수 */
void testServer(type_server& test)
{
    for (auto& s : test)
        std::cout << "key:" << std::setw(15) << s.first  << "||  value:" << s.second << std::endl;
    std::cout << "=====================================" << std::endl;
}

void testLocation(std::map<std::string, type_location>& test)
{
    for (auto& a : test)
    {
        std::cout << "route: " << a.first << std::endl;
        type_location temp = a.second;
        for (auto& b : temp)
        {
            std::cout << "key: " << std::setw(15) << b.first <<  "||  value: " << b.second << std::endl;
        }
        std::cout << "=====================================" << std::endl;
    }
}
