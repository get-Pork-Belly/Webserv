#include "ServerManager.hpp"
#include "Server.hpp"
#include "utils.hpp"

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

void
ServerManager::setFdMax(int fd)
{
    this->_fd_max = fd;
}

/*============================================================================*/
/******************************  Exception  ***********************************/
/*============================================================================*/

/*============================================================================*/
/*********************************  Util  *************************************/
/*============================================================================*/

void
ServerManager::fdSet(int fd, int type)
{
    if (type == READ_FDSET)
        ft::fdSet(fd, &this->_readfds);
    else if (type == WRITE_FDSET)
        ft::fdSet(fd, &this->_writefds);
    else if (type == EXCEPT_FDSET)
        ft::fdSet(fd, &this->_exceptfds);
    else
    {
        ft::fdSet(fd, &this->_readfds);
        ft::fdSet(fd, &this->_writefds);
        ft::fdSet(fd, &this->_exceptfds);
    }
}

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
        ft::fdClr(fd, &this->_readfds);
    else if (type == WRITE_FDSET)
        ft::fdClr(fd, &this->_writefds);
    else if (type == EXCEPT_FDSET)
        ft::fdClr(fd, &this->_exceptfds);
}

/*============================================================================*/
/************************  Manage Server functions  ***************************/
/*============================================================================*/


void
ServerManager::initServers()
{
    ServerGenerator server_generator(this);
    server_generator.generateServers(this->_servers);
}

bool
ServerManager::runServers()
{
    int selected_fds;
    struct timeval timeout;

    timeout.tv_sec = 5;
    timeout.tv_usec = 5;
    for (Server *server : this->_servers)
    {
        int server_socket = server->getServerSocket();
        this->fdSet(server_socket, ALL_FDSET);
        this->setFdMax(server_socket);
    }
    //TODO: siganl 입력시 반복종료 구현
    while (true)
    {
        this->_copy_readfds = this->_readfds;
        this->_copy_writefds = this->_writefds;
        this->_copy_exceptfds = this->_exceptfds;
        if ((selected_fds = select(this->getFdMax() + 1,
             &this->_copy_readfds, &this->_copy_writefds,
             &this->_copy_exceptfds, &timeout)) == -1)
        {
            std::cerr<<"Error : select"<<std::endl;
            return (false);
        }
        else if (selected_fds == 0)
        {
            std::cout<<"Time Out"<<std::endl;
            continue ;
        }
        else
        {
            for (int fd = 0; fd < this->getFdMax() + 1; fd++)
            {
                if (this->fdIsSet(fd, ALL_FDSET))
                {
                    for (Server *server : this->_servers)
                    {
                        if (fd == server->getServerSocket() ||
                            server->isClientOfServer(fd))
                            server->run(this, fd);
                    }
                }
            }
        }
    }
    return (true);
}




// void
// ServerManager::exitServers()
// {
    
// }
