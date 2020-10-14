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
    std::vector<std::string> lines(100);

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
            this->_configfile_lines.push_back(std::string(trimmed));
    }
}

/*
 * http{
 *   root = /user/;
 *
 *   server {
 *		root = /user/sanam/;
 *   } -> server_confing -> servers.push_back(new Server(server_config))
 *
 *   server {
 *   } -> server_confing -> servers.push_back(new Server(server_config))
 * }
 *
 * _server_config[root] = /user;
 * _server_config[root] = /user/sanam/으로 갱신 - 구체화된다.
 */
// NOTE 함수 내부에서 parse 함수들을 호출한다.

void
ServerGenerator::generateServers(std::vector<Server *>& servers)
{
    std::vector<std::string> directives;
    std::map<std::string, std::string> _http_config;
    
    std::vector<std::string>::iterator it = this->_configfile_lines.begin();
    std::vector<std::string>::iterator ite = this->_configfile_lines.end();
    //TODO http_config 를 먼저 작성하자.
    // std::map<std::string, std::string> _http = httpParseBlock();
    (void)servers;

    while (it != ite)
    {
        if ( *it == "server {")
        {
            std::map<std::string, std::string> _server_config(_http_config);
            it++;
            // std::map<std::string, std::map> _locations;
            // parseServerBlock(it, _server_config, _locations);
            parseServerBlock(it, _server_config);
            //TODO  new로 해야할까?
            // servers.push_back(new Server(_server_config));
            // servers.push_back(new Server(_server_config, _locations));
        }
        it++;
    }
}


// void
// ServerGenerator::parseServerBlock(std::vector<std::string>::iterator& it, std::map<std::string, std::string>& server_config
// , std::map<std::string, std::map> locations)
void
ServerGenerator::parseServerBlock(std::vector<std::string>::iterator& it, std::map<std::string, std::string>& server_config)
{
    std::vector<std::string>	directives;

    while (it != _configfile_lines.end())
    {
        directives = ft::split(*it, " "); //NOTE directives[0]: key, [1]: value
        if (directives[0] == "location")
        {
            // std::map<std::string, std::string> location_config =  parseLocationBlock(it, server_config);
            // locations.[location_config['Routing Table']] = location_config;
            parseLocationBlock(it, server_config); //NOTE server_config에서 location 설정 처리도 같이하는건 어떨까?
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

// std::map<std::string, std::string>
// ServerGenerator::parseLocationBlock(std::vector<std::string>::iterator& it, std::map<std::string, std::string>& server_config)
void
ServerGenerator::parseLocationBlock(std::vector<std::string>::iterator& it, std::map<std::string, std::string>& server_config)
{
    std::vector<std::string> directives;
    
    // std::map<std::string, std::string> location_config;
    // setLocationConfig(location_config, server_config);
    while (it != _configfile_lines.end())
    {
        directives = ft::split(*it, " ");
        if (directives[0] == "}")
        {
            it++;
            return ;
        }
        std::string temp = ft::rtrim(directives[1], ";");
        directives[1] = temp;
        server_config[directives[0]] = directives[1];
        directives.clear();
        it++;
    }
}
