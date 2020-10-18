#include "ServerManager.hpp"

/*============================================================================*/
/****************************  Static variables  ******************************/
/*============================================================================*/

/*============================================================================*/
/******************************  Constructor  *********************************/
/*============================================================================*/

ServerManager::ServerManager(const char *config_path)
: _config_file_path(config_path)
{
    this->initServers();

    //TODO: FD_ZERO 구현
    FD_ZERO(&this->_readfds);
    FD_ZERO(&this->_writefds);
    FD_ZERO(&this->_exceptfds);
    FD_ZERO(&this->_copy_readfds);
    FD_ZERO(&this->_copy_writefds);
    FD_ZERO(&this->_copy_exceptfds);

    //TODO: 임시로 초기화. 수정 필요
    // this->_servers = 0;
    this->_port = "default";
    this->_fd = 0;
    this->_fd_max = 0;

    this->_status_code_msg = {
        {101, "wowowo"},
        {102, "wowowo"}
    };
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

const char *
ServerManager::getConfigFilePath() const
{
    return (this->_config_file_path);
}

const int
ServerManager::getFdMax() const
{
    return (this->_fd_max);
}

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
    ServerGenerator server_generator(this);
    server_generator.generateServers(this->_servers); // config파일을 순회하며 Server객체 생성 _servers.push_b
    
    // TODO Server가 먼저 구현되어야만 한다.
    // for (Server *server: this->_servers)
    // {
    //     struct sockaddr_in addr;
    //     if (bind(server->getServerSocket(), (struct sockaddr *)&addr, sizeof(addr)) == -1)
    //         throw "Bind Error"; //NOTE: exception instance 만들기
    //
    //     if (listen(server->getServerSocket(), 1024) == -1)
    //         throw "Listen Error"; //NOTE: exception instance 만들기
    // }
}

bool
ServerManager::runServers()
{
    int selected_fds;
    struct timeval timeout;

    this->_copy_readfds = this->_readfds;
    this->_copy_writefds = this->_writefds;
    this->_copy_exceptfds = this->_exceptfds;
    timeout.tv_sec = 5;
    timeout.tv_usec = 5;

    //TODO: siganl 입력시 반복종료 구현
    while (true)
    {
        if ((selected_fds = select(this->getFdMax() + 1, &this->_copy_readfds, &this->_copy_writefds, &this->_copy_exceptfds, &timeout)) == -1)
        {
            std::cerr<<"Error : select"<<std::endl;
            return (false);
        }
        else if (selected_fds == 0)
            continue ;
        for (Server *server : this->_servers)
            server->run();
    }
    return (true);
}

// void
// ServerManager::exitServers()
// {
    
// }
