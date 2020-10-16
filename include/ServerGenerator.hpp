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

class ServerManager;

const int BUF_SIZE = 4096;

class ServerGenerator
{
private:
    ServerGenerator();
    ServerGenerator(const ServerGenerator& other);
    ServerGenerator& operator=(const ServerGenerator& rhs);

private:
    const ServerManager*		_server_manager;
    std::vector<std::string>	_configfile_lines;

public:
    /* Constructor */
    ServerGenerator(const ServerManager* _server_manager);

    /* Destructor */
    virtual ~ServerGenerator();

    /* Overload */

    /* Getter */
    /* Setter */
    /* Exception */
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
};

void testLocation(std::map<std::string, location_info>& test);
void testServer(server_info& test);

#endif
