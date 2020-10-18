#include "ServerManager.hpp"
#include "Server.hpp"

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
    ft::fdZero(&this->_readfds);
    ft::fdZero(&this->_writefds);
    ft::fdZero(&this->_exceptfds);
    ft::fdZero(&this->_copy_readfds);
    ft::fdZero(&this->_copy_writefds);
    ft::fdZero(&this->_copy_exceptfds);

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

int
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

    timeout.tv_sec = 5;
    timeout.tv_usec = 5;

    (void)selected_fds;
    //TODO: siganl 입력시 반복종료 구현
    while (true)
    {
        // this->_copy_readfds = this->_readfds;
        // this->_copy_writefds = this->_writefds;
        // this->_copy_exceptfds = this->_exceptfds;
        // if ((selected_fds = select(this->getFdMax() + 1, &this->_copy_readfds, &this->_copy_writefds, &this->_copy_exceptfds, &timeout)) == -1)
        // {
        //     std::cerr<<"Error : select"<<std::endl;
        //     return (false);
        // }
        // else if (selected_fds == 0)
        //     continue ;
        for (Server *server : this->_servers)
        {
            // server->run(this);
            server->test(this);
        }
    }
    return (true);
}

// void
// ServerManager::exitServers()
// {
    
// }
