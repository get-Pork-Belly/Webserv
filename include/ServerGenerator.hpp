#ifndef SERVERGENERATOR_HPP
# define SERVERGENERATOR_HPP

# include <vector>
# include <map>
# include <string>
# include <iostream>
# include "utils.hpp"
# include "Server.hpp"

class ServerManager;

// TODO 아마 이 부분은 컴파일 에러가 뜰 수 있음. -> 다중 선언?
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
    // NOTE it를 레퍼런스로 넣어주는 이유는 it의 값을 다른 함수에서도 일괄적으로 움직일 수 있게 하기 위함.
    // server_config의 값을 레퍼런스로 주는 이유는 generateServers에 server_config 변수가 선언되어 있기 때문.
    // void parseServerBlock(std::vector<std::string>::iterator& it, std::map<std::string, std::string>& server_config); // 이후에 private로 바꿔주자.
    void parseServerBlock(std::vector<std::string>::iterator& it, std::map<std::string, std::string>& server_config); 
    void parseLocationBlock(std::vector<std::string>::iterator& it, std::map<std::string, std::string>& server_config);
};

#endif
