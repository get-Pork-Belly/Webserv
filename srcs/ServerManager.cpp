#include "ServerManager.hpp"
#include "Server.hpp"
#include "utils.hpp"

/*============================================================================*/
/****************************  Static variables  ******************************/
/*============================================================================*/

/*============================================================================*/
/******************************  Constructor  *********************************/
/*============================================================================*/

ServerManager::ServerManager(const char* config_path)
: _config_file_path(config_path)
{
    ft::fdZero(&this->_readfds);
    ft::fdZero(&this->_writefds);
    ft::fdZero(&this->_exceptfds);
    ft::fdZero(&this->_copy_readfds);
    ft::fdZero(&this->_copy_writefds);
    ft::fdZero(&this->_copy_exceptfds);
    this->_port = "default";
    // this->_fd_table.resize(1024, FdType::CLOSED);

    this->_fd_table.resize(1024, std::pair<FdType, int>(FdType::CLOSED, -1));
    this->_fd = 0;
    this->_fd_max = 2;
    this->initServers();
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

const std::vector<std::pair<FdType, int> >& 
ServerManager::getFdTable() const
{
    return (this->_fd_table);
}

FdType
ServerManager::getFdType(int fd) const
{
    const std::pair<FdType, int>& pair = _fd_table.at(fd);
    return (pair.first);
}

int
ServerManager::getConnectedFd(int fd) const
{
    return (this->_fd_table[fd].second);
}
/*============================================================================*/
/********************************  Setter  ************************************/
/*============================================================================*/

void
ServerManager::setFdMax(int fd)
{
    this->_fd_max = fd;
}

void
ServerManager::setServerSocketOnFdTable(int fd)
{
    this->_fd_table[fd].first = FdType::SERVER_SOCKET;
    this->_fd_table[fd].second = -1;
}

void
ServerManager::setClientSocketOnFdTable(int fd, int server_socket)
{
    this->_fd_table[fd].first = FdType::CLIENT_SOCKET;
    this->_fd_table[fd].second = server_socket;
}

void
ServerManager::setResourceOnFdTable(int fd, int client_socket)
{
    this->_fd_table[fd].first = FdType::RESOURCE;
    this->_fd_table[fd].second = client_socket;
}

void
ServerManager::setCGIPipeOnFdTable(int fd, int client_socket)
{
    this->_fd_table[fd].first = FdType::PIPE;
    this->_fd_table[fd].second = client_socket;
}

void
ServerManager::setClosedFdOnFdTable(int fd)
{
    this->_fd_table[fd].first = FdType::CLOSED;
}

/*============================================================================*/
/******************************  Exception  ***********************************/
/*============================================================================*/

/*============================================================================*/
/*********************************  Util  *************************************/
/*============================================================================*/

void
ServerManager::fdSet(int fd, FdSet type)
{
    switch (type)
    {
    case FdSet::READ:
        ft::fdSet(fd, &this->_readfds);
        break;

    case FdSet::WRITE:
        ft::fdSet(fd, &this->_writefds);
        break;

    case FdSet::EXCEPT:
        ft::fdSet(fd, &this->_exceptfds);
        break;
    
    default:
        ft::fdSet(fd, &this->_readfds);
        ft::fdSet(fd, &this->_writefds);
        ft::fdSet(fd, &this->_exceptfds);
        break;
    }
}

bool
ServerManager::fdIsSet(int fd, FdSet type)
{
    switch (type)
    {
    case FdSet::READ:
        return (ft::fdIsSet(fd, &this->_copy_readfds));

    case FdSet::WRITE:
        return (ft::fdIsSet(fd, &this->_copy_writefds));

    case FdSet::EXCEPT:
        return (ft::fdIsSet(fd, &this->_copy_exceptfds));

    default:
        return (ft::fdIsSet(fd, &this->_copy_readfds) ||
        ft::fdIsSet(fd, &this->_copy_writefds) ||
        ft::fdIsSet(fd, &this->_copy_exceptfds));
    }
}

void
ServerManager::fdClr(int fd, FdSet type)
{
    switch (type)
    {
    case FdSet::READ:
        ft::fdClr(fd, &this->_readfds);
        break;

    case FdSet::WRITE:
        ft::fdClr(fd, &this->_writefds);
        break;
    
    case FdSet::EXCEPT:
        ft::fdClr(fd, &this->_exceptfds);
        break;

    default:
        ft::fdClr(fd, &this->_readfds);
        ft::fdClr(fd, &this->_writefds);
        ft::fdClr(fd, &this->_exceptfds);
        break;
    }
}

void
ServerManager::updateFdMax(int fd)
{
    switch (this->_fd_table[fd].first)
    {
    case FdType::CLOSED:
        if (this->_fd_max == fd)
        {
            for (int i = fd - 1; i > 2; i--)
            {
                if (this->_fd_table[i].first != FdType::CLOSED)
                {
                    this->setFdMax(i);
                    break ;
                }
            }
        }
        break;

    default:
        if (this->_fd_max < fd)
            this->setFdMax(fd);
        break;
    }
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
        this->fdSet(server_socket, FdSet::ALL);
        this->updateFdMax(server_socket);
    }
    //TODO: siganl 입력시 반복종료 구현
    while (true)
    {
        this->_copy_readfds = this->_readfds;
        this->_copy_writefds = this->_writefds;
        this->_copy_exceptfds = this->_exceptfds;
        if ((selected_fds = select(this->getFdMax() + 1, &this->_copy_readfds, 
            &this->_copy_writefds, &this->_copy_exceptfds, &timeout)) == -1)
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
                if (this->fdIsSet(fd, FdSet::ALL))
                {
                    for (Server *server : this->_servers)
                    {
                        if (fd == server->getServerSocket() || server->isFdManagedByServer(fd))
                            server->run(fd);
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
