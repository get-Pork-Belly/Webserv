#ifndef SERVERGENERATOR_HPP
# define SERVERGENERATOR_HPP

# include <vector>
# include <map>
# include <string>
# include <iostream>
# include "types.hpp"
# include "utils.hpp"
# include "Server.hpp"
# include <iomanip>
# include "exception"

class ServerManager;

const int BUF_SIZE = 4096;
const std::string NO_SEMICOLON = "Directive must end with a \';\'";
const std::string INVALID_BLOCK = "Block directive needs one arguments";

class ServerGenerator
{
private:
    ServerGenerator();
    ServerGenerator(const ServerGenerator& other);
    ServerGenerator& operator=(const ServerGenerator& rhs);

private:
    ServerManager*		_server_manager;
    std::vector<std::string>	_configfile_lines;

public:
    /* Constructor */
    ServerGenerator(ServerManager* _server_manager);

    /* Destructor */
    virtual ~ServerGenerator();

    /* Overload */

    /* Getter */
    /* Setter */
    /* Exception */
    class ConfigFileSyntaxError : public ConfigFileErrorException
    {
        private:
            std::string _msg;
        public:
            ConfigFileSyntaxError(std::string msg);
            virtual const char* what() const throw();
    };
    /* Util */
    void convertFileToStringVector(const char *config_file_path);
    // TODO isValidConfigFile 구현하기
    // bool isValidConfigFile() const; //throw
    void generateServers(std::vector<Server *>& servers);

    void initHttpConfig(server_info& http_config);
    void initServerConfig(
            server_info& server_config,
            server_info& http_config);

    void initLocationConfig(
            location_info& location_config,
            server_info& server_config);

    server_info parseHttpBlock();
    void parseServerBlock(
            std::vector<std::string>::iterator& it,
            server_info& server_config,
            std::map<std::string,
            location_info>& locations);

    location_info parseLocationBlock(
            std::vector<std::string>::iterator& it,
            location_info& server_config);

    void (ServerGenerator::*skipServerBlock)(
         std::vector<std::string>::iterator&,
         server_info&,
         std::map<std::string, location_info>&) = &ServerGenerator::parseServerBlock;

    void setDefaultRouteOfServer(std::map<std::string,
            location_info>& locations, server_info& server_config);
};

void testLocationConfig(std::map<std::string, location_info>& test);
void testServerConfig(server_info& test);

#endif
