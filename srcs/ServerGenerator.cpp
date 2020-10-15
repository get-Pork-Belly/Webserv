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
    std::vector<std::string> directives;
    std::map<std::string, std::string> http_config;
    
    std::vector<std::string>::iterator it = this->_configfile_lines.begin();
    std::vector<std::string>::iterator ite = this->_configfile_lines.end();
    // std::map<std::string, std::string> _http = httpParseBlock();
    (void)servers;

    while (it != ite)
    {
        if ( *it == "server {")
        {
            std::map<std::string, std::string> server_config(http_config);
            it++;
            std::map<std::string, std::map<std::string, std::string> > locations;
            parseServerBlock(it, server_config, locations);
            
            std::cout << "===============================" << std::endl;
            for (auto& s : server_config)
            {
                std::cout << "key: " << s.first << "  value: " << s.second << std::endl;
            }
            std::cout << "==========   locations ========" << std::endl;
            for (auto& s : locations)
            {
                std::cout << "---------------------------" << std::endl;
                std::cout << "route: " << s.first << std::endl;
                for (auto& b : s.second)
                {
                    std::cout << "key: " << b.first << "  value: " << b.second << std::endl;
                }
                std::cout << "-----------다음----------------" << std::endl;
            }
            // servers.push_back(new Server(server_config, locations));
            std::cout << "===============================\n\n" << std::endl;
        }
        it++;
    }
}

void
ServerGenerator::parseServerBlock(std::vector<std::string>::iterator& it, std::map<std::string, std::string>& server_config, std::map<std::string, std::map<std::string, std::string> >& locations)
{
    std::vector<std::string>	directives;

    while (it != _configfile_lines.end())
    {
        directives = ft::split(*it, " ");
        if (directives[0] == "location")
        {
            std::map<std::string, std::string> location_config = parseLocationBlock(it, server_config);
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

void
ServerGenerator::setLocationConfig(std::map<std::string, std::string>& location_config, std::map<std::string, std::string>& server_config)
{
    std::map<std::string, std::string>::iterator ite = server_config.end();

    if (server_config.find("index") != ite)
        location_config["index"] = server_config["index"];
    if (server_config.find("root") != ite)
        location_config["root"] = server_config["root"];
}

std::map<std::string, std::string>
ServerGenerator::parseLocationBlock(std::vector<std::string>::iterator& it, std::map<std::string, std::string>& server_config)
{
    std::vector<std::string> directives;
    std::map<std::string, std::string> location_config;

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
        std::string temp = ft::rtrim(directives[1], ";");
        directives[1] = temp;
        location_config[directives[0]] = directives[1];
        directives.clear();
        it++;
    }
    return (location_config);
}
