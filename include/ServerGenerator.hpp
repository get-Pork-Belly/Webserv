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
    void setGlobalConfig();
    void generateServers(std::vector<Server *>& servers);

    void parseServerBlock(std::vector<std::string>::iterator& it, type_server& server_config, std::map<std::string, type_location>& locations);

    type_location parseLocationBlock(std::vector<std::string>::iterator& it, type_location& server_config);

    void setLocationConfig(type_location& location_config, type_server& server_config);

    type_server parseHttpBlock();
};
void testLocation(std::map<std::string, type_location>& test);
void testServer(type_server& test);

#endif
