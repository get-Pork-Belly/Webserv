#include "ServerManager.hpp"
#include "Server.hpp"
#include "utils.hpp"
#include "Log.hpp"

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

    this->_fd_table.resize(1024, std::pair<FdType, int>(FdType::CLOSED, DEFAULT_FD));
    this->_last_request_time_of_client.resize(1024, std::pair<MonitorStatus, timeval>(false, timeval()));
    this->_fd = DEFAULT_FD;
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
ServerManager::getLinkedFdFromFdTable(int fd) const
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
    this->_fd_table[fd].second = DEFAULT_FD;
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
ServerManager::fdIsCopySet(int fd, FdSet type)
{
    switch (type)
    {
    case FdSet::READ:
        return (ft::fdIsCopySet(fd, &this->_copy_readfds));

    case FdSet::WRITE:
        return (ft::fdIsCopySet(fd, &this->_copy_writefds));

    case FdSet::EXCEPT:
        return (ft::fdIsCopySet(fd, &this->_copy_exceptfds));

    default:
        return (ft::fdIsCopySet(fd, &this->_copy_readfds) ||
        ft::fdIsCopySet(fd, &this->_copy_writefds) ||
        ft::fdIsCopySet(fd, &this->_copy_exceptfds));
    }
}

//NOTE: log 함수 출력을 위한 함수
bool
ServerManager::fdIsOriginSet(int fd, FdSet type)
{
    switch (type)
    {
    case FdSet::READ:
        return (ft::fdIsCopySet(fd, &this->_readfds));

    case FdSet::WRITE:
        return (ft::fdIsCopySet(fd, &this->_writefds));

    case FdSet::EXCEPT:
        return (ft::fdIsCopySet(fd, &this->_exceptfds));

    default:
        return (ft::fdIsCopySet(fd, &this->_readfds) ||
        ft::fdIsCopySet(fd, &this->_writefds) ||
        ft::fdIsCopySet(fd, &this->_exceptfds));
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
    signal(SIGPIPE, SIG_IGN);
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
        this->closeUnresponsiveClient();

        this->_copy_readfds = this->_readfds;
        this->_copy_writefds = this->_writefds;
        this->_copy_exceptfds = this->_exceptfds;

        // std::cout<<"\033[1;44;37m"<<"Before select!"<<"\033[0m"<<std::endl;
        // Log::printFdCopySets(*this);
        // Log::printFdSets(*this);
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
        // std::cout<<"\033[1;44;37m"<<"After select!"<<"\033[0m"<<std::endl;
        // Log::printFdCopySets(*this);
        // Log::printFdSets(*this);
            for (int fd = 0; fd < this->getFdMax() + 1; fd++)
            {
                if (this->fdIsCopySet(fd, FdSet::ALL))
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

void
ServerManager::setLastRequestTimeOfClient(int client_fd, MonitorStatus check, timeval* time)
{
    this->_last_request_time_of_client[client_fd].first = check;
    if (time != NULL)
        this->_last_request_time_of_client[client_fd].second = *time;
}

bool
ServerManager::isClientTimeOut(int fd)
{
    timeval now;
    gettimeofday(&now, NULL);
    
    std::cout<<"\033[1;36m"<<"> in isClientTimeOut"<<"\033[0m"<<std::endl;
    if (this->_last_request_time_of_client[fd].first == false)
    {
        std::cout<<"\033[1;36m"<<"> in isClientTimeOut false 1"<<"\033[0m"<<std::endl;
        return (false);
    }
    if (now.tv_sec - this->_last_request_time_of_client[fd].second.tv_sec > TIME_OUT_SECOND)
    {
        std::cout<<"\033[1;36m"<<"> in isClientTimeOut true"<<"\033[0m"<<std::endl;
        return (true);
    }

        std::cout<<"\033[1;36m"<<"> in isClientTimeOut false 2"<<"\033[0m"<<std::endl;
    return (false);
}

void
ServerManager::monitorTimeOutOff(int fd)
{
    this->_last_request_time_of_client[fd].first = false;
}

void
ServerManager::monitorTimeOutOn(int fd)
{
    this->_last_request_time_of_client[fd].first = true;
    gettimeofday(&(this->_last_request_time_of_client[fd].second), NULL);
}

bool
ServerManager::isMonitorTimeOutOn(int fd)
{
    if (this->_last_request_time_of_client[fd].first == true)
        std::cout<<"\033[1;30;43m"<<"in isMoniotrTimeOutOn: ";
        Log::printTimeSec(this->_last_request_time_of_client[fd].second);
        std::cout<<"\033[0m"<<std::endl;
    return (this->_last_request_time_of_client[fd].first);
}

void
ServerManager::closeUnresponsiveClient()
{
    for (int fd = 3; fd < this->getFdMax() + 1; fd++)
    {
        if (this->fdIsOriginSet(fd, FdSet::READ) &&
            this->getFdType(fd) == FdType::CLIENT_SOCKET &&
            this->fdIsOriginSet(fd, FdSet::READ) != this->fdIsCopySet(fd, FdSet::READ))
        {
            // Log::printFdSets(*this);
            // Log::printFdCopySets(*this);
            if (this->isMonitorTimeOutOn(fd))
            {
                if (this->isClientTimeOut(fd))
                {
                    this->fdSet(fd, FdSet::WRITE);
                    for (Server* server : this->_servers)
                    {
                        if (server->getServerSocket() == this->getFdTable()[fd].second)
                        {
                            Response& response = server->getResponse(fd);
                            response.setStatusCode("408");
                            if (response.getWriteFdToCGI() != DEFAULT_FD ||
                                response.getReadFdFromCGI() != DEFAULT_FD)
                            {
                                server->closeFdAndUpdateFdTable(response.getReadFdFromCGI(), FdSet::READ);
                                server->closeFdAndUpdateFdTable(response.getWriteFdToCGI(), FdSet::WRITE);
                            }
                            else if (response.getResourceFd() != DEFAULT_FD)
                                server->closeFdAndUpdateFdTable(response.getResourceFd(), FdSet::READ);
                        }
                    }
                    this->monitorTimeOutOff(fd);
                }
            }
            else
                this->monitorTimeOutOn(fd);
        }
        else
            this->monitorTimeOutOff(fd);
    }
}

// void
// ServerManager::exitServers()
// {
    
// }
