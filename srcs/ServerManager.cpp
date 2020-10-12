#include "ServerManager.hpp"

/*============================================================================*/
/****************************  Static variables  ******************************/
/*============================================================================*/

/*============================================================================*/
/******************************  Constructor  *********************************/
/*============================================================================*/

ServerManager::ServerManager(const char *config_path)
: _config_file_path(std::string(config_path))
{
    this->initServers();

    //TODO: FD_ZERO 구현
    FD_ZERO(this->_readfds);
    FD_ZERO(this->_writefds);
    FD_ZERO(this->_exceptfds);
    FD_ZERO(this->_copy_readfds);
    FD_ZERO(this->_copy_writefds);
    FD_ZERO(this->_copy_exceptfds);

    //TODO: status_code_msg 초기화
    this->_status_code_msg = {
        {101, "wowowo"},
        {102, "wowowo"}
    };
    memset(&this->_global_config, 0, sizeof(GlobalConfig));
}

/*============================================================================*/
/******************************  Destructor  **********************************/
/*============================================================================*/

ServerManager::~ServerManager()
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
ServerManager::initServers()
{
    ServerGenerator server_generator;

    server_generator.convertFileToString(this->_config_file_path);
    if (!server_generator.isValidConfigFile())
        throw "config file error"; // throw
    this->_global_config = server_generator.setGlobalConfig(); // GlobalConfig set

    server_generator.generateServers(this->_servers); // config파일을 순회하며 Server객체 생성 _servers.push_b
    
    // server_generator.parseServerBlock(); - generateServers 내부에서 실행
    // server_generator.parseLocationBlock(); - generateServers 내부에서 실행

    for (Server *server: this->_servers)
    {
        struct sockaddr_in addr;
        if (bind(server->getServerSocket(), (struct sockaddr *)&addr, sizeof(addr)) == -1)
            throw "Bind Error"; //NOTE: exception instance 만들기

        if (listen(server->getServerSocket(), 1024) == -1)
            throw "Listen Error"; //NOTE: exception instance 만들기
    }
}

bool
ServerManager::runServers()
{
    
}

void
ServerManager::exitServers()
{
    
}