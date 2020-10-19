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

bool
ServerManager::fdIsSet(int fd, int type)
{
    if (type == READ_FDSET)
        return (ft::fdIsSet(fd, &this->_copy_readfds));
    else if (type == WRITE_FDSET)
        return (ft::fdIsSet(fd, &this->_copy_writefds));
    else if (type == EXCEPT_FDSET)
        return (ft::fdIsSet(fd, &this->_copy_exceptfds));
    else
    {
        return (ft::fdIsSet(fd, &this->_copy_readfds) ||
        ft::fdIsSet(fd, &this->_copy_writefds) ||
        ft::fdIsSet(fd, &this->_copy_exceptfds));
    }
}

void
ServerManager::fdClr(int fd, int type)
{
    if (type == READ_FDSET)
        ft::fdClr(fd, &this->_copy_readfds);
    else if (type == WRITE_FDSET)
        ft::fdClr(fd, &this->_copy_writefds);
    else if (type == EXCEPT_FDSET)
        ft::fdClr(fd, &this->_copy_exceptfds);
}

/*============================================================================*/
/************************  Manage Server functions  ***************************/
/*============================================================================*/


void
ServerManager::initServers()
{
    ServerGenerator server_generator(this);
    server_generator.generateServers(this->_servers); // config파일을 순회하며 Server객체 생성 _servers.push_b
}

bool
ServerManager::runServers()
{
    int selected_fds;
    struct timeval timeout;

    timeout.tv_sec = 5;
    timeout.tv_usec = 5;

    //TODO: siganl 입력시 반복종료 구현
    while (true)
    {
        this->_copy_readfds = this->_readfds;
        this->_copy_writefds = this->_writefds;
        this->_copy_exceptfds = this->_exceptfds;
        if ((selected_fds = select(this->getFdMax() + 1, &this->_copy_readfds, &this->_copy_writefds, &this->_copy_exceptfds, &timeout)) == -1)
        {
            std::cerr<<"Error : select"<<std::endl;
            return (false);
        }
        else if (selected_fds == 0)
            continue ;
        std::cout << "in while" << std::endl;
        for (Server *server : this->_servers)
            server->run(this);
    }
    return (true);
}












































// void
// ServerManager::exitServers()
// {
    
// }
