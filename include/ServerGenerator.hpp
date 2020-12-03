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
const std::string INVALID_BLOCK = "Block directive's syntax error";
const std::string BRACKET_ERROR = "Unexpected end of file, expecting \"}\"";
const std::string NO_HTTP_BLOCK = "There must be http block in config file";
const std::string NO_ARGUMENT = "Simple directive have at least one arguments";
const std::string INVALID_ARGUMENTS = "Invalid arguments at Simple directive";
const std::string NOT_VALID_DIRECTIVE = "is not a valid Directive";

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
    void checkHttpBlock(std::vector<std::string>::iterator& it, const std::vector<std::string>::iterator& ite);
    bool isEmptyLine(const std::string& line);
    void setDirectiveToConfig(std::map<std::string, std::string>& config, std::vector<std::string>& directive);
    void validCheckOfLocations(std::map<std::string, location_info>& locations);
    void validCheckOfServer(server_info& server);

};

void testLocationConfig(std::map<std::string, location_info>& test);
void testServerConfig(server_info& test);

#endif
