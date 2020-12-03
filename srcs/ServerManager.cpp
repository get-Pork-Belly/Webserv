#include "ServerManager.hpp"
#include "Server.hpp"
#include "utils.hpp"
#include "Log.hpp"

/*============================================================================*/
/****************************  Static variables  ******************************/
/*============================================================================*/

extern std::vector<int> g_child_process_ids;

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
    this->_last_update_time_of_fd.resize(1024, std::pair<MonitorStatus, timeval>(false, timeval()));
    this->_fd = DEFAULT_FD;
    this->_fd_max = 2;
    this->initServers();
}

ServerManager&
ServerManager::operator=(const ServerManager& rhs)
{
    this->_config_file_path = rhs._config_file_path;
    this->_servers = rhs._servers;
    this->_readfds = rhs._readfds;
    this->_writefds = rhs._writefds;
    this->_exceptfds = rhs._exceptfds;
    this->_copy_readfds = rhs._copy_readfds;
    this->_copy_writefds = rhs._copy_writefds;
    this->_copy_exceptfds = rhs._copy_exceptfds;
    this->_port = rhs._port;
    this->_fd_table = rhs._fd_table;
    this->_fd = rhs._fd;
    this->_fd_max = rhs._fd_max;
    this->_last_update_time_of_fd = rhs._last_update_time_of_fd;
    return (*this);
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
ServerManager::setCgiPipeOnFdTable(int fd, int client_socket)
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
    ServerGenerator server_generator(this);
    server_generator.generateServers(this->_servers);
}

bool
ServerManager::runServers()
{
    int selected_fds;
    struct timeval timeout;

    signal(SIGINT, exitServers);
    signal(SIGPIPE, SIG_IGN);

    timeout.tv_sec = 5;
    timeout.tv_usec = 5;
    for (Server *server : this->_servers)
    {
        int server_socket = server->getServerSocket();
        this->fdSet(server_socket, FdSet::ALL);
        this->updateFdMax(server_socket);
    }
    while (true)
    {
        this->closeUnresponsiveFd();

        // std::cout<<"\033[1;44;37m"<<"BEFORE select!"<<"\033[0m"<<std::endl;
        // Log::printFdCopySets(*this, 10);
        // Log::printFdSets(*this, 10);
        this->_copy_readfds = this->_readfds;
        this->_copy_writefds = this->_writefds;
        this->_copy_exceptfds = this->_exceptfds;

        if ((selected_fds = select(this->getFdMax() + 1, &this->_copy_readfds, 
            &this->_copy_writefds, &this->_copy_exceptfds, &timeout)) == -1)
        {
            this->clearServers();
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
        // Log::printFdCopySets(*this, 10);
        // Log::printFdSets(*this, 10);
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
ServerManager::setLastUpdateTimeOfFd(int fd, MonitorStatus check, timeval* time)
{
    this->_last_update_time_of_fd[fd].first = check;
    if (time != NULL)
        this->_last_update_time_of_fd[fd].second = *time;
}

bool
ServerManager::isFdTimeOut(int fd)
{
    timeval now;
    gettimeofday(&now, NULL);
    
    if (this->_last_update_time_of_fd[fd].first == false)
        return (false);

    switch (this->getFdType(fd))
    {
    case FdType::CLIENT_SOCKET:
        if (now.tv_sec - this->_last_update_time_of_fd[fd].second.tv_sec > CLIENT_TIME_OUT_SECOND)
            return (true);
    
    case FdType::PIPE:
        if (now.tv_sec - this->_last_update_time_of_fd[fd].second.tv_sec > CGI_TIME_OUT_SECOND)
            return (true);

    default:
        break;
    }
    return (false);
}

void
ServerManager::monitorTimeOutOff(int fd)
{
    this->_last_update_time_of_fd[fd].first = false;
}

void
ServerManager::monitorTimeOutOn(int fd)
{
    this->_last_update_time_of_fd[fd].first = true;
    gettimeofday(&(this->_last_update_time_of_fd[fd].second), NULL);
}

bool
ServerManager::isMonitorTimeOutOn(int fd)
{
    return (this->_last_update_time_of_fd[fd].first);
}

void
ServerManager::closeUnresponsiveClient(int client_fd)
{
    if (this->isMonitorTimeOutOn(client_fd))
    {
        if (this->isFdTimeOut(client_fd))
        {
            Server* server = findLinkedServer(client_fd);
            if (!server)
                return ;
            if (server->getServerSocket() == this->getFdTable()[client_fd].second)
                server->getResponse(client_fd).setStatusCode("408");
            this->fdSet(client_fd, FdSet::WRITE);
            this->monitorTimeOutOff(client_fd);
        }
    }
    else
        this->monitorTimeOutOn(client_fd);
}

void
ServerManager::closeUnresponsiveCgi(int pipe_fd)
{
    if (this->isMonitorTimeOutOn(pipe_fd))
    {
        if (this->isFdTimeOut(pipe_fd))
        {
            int client_fd = this->getLinkedFdFromFdTable(pipe_fd);
            Server* server = findLinkedServer(client_fd);
            if (!server)
                return ;
            Response& response = server->getResponse(client_fd);
            kill(response.getCgiPid(), SIGKILL);
            g_child_process_ids.erase(std::remove(g_child_process_ids.begin(), g_child_process_ids.end(),
                                        response.getCgiPid()), g_child_process_ids.end());
            if (response.getSendProgress() == SendProgress::READY)
                response.setStatusCode("500");
            else
            {
                response.setTransmittingBody("0\r\n\r\n");
                response.setParseProgress(ParseProgress::FINISH);
                if (response.getWriteFdToCgi() != DEFAULT_FD)
                    this->closeCgiWritePipe(*server, response.getWriteFdToCgi());
                if (response.getReadFdFromCgi() != DEFAULT_FD)
                    this->closeCgiReadPipe(*server, response.getReadFdFromCgi());
                if (response.getResourceFd() != DEFAULT_FD)
                    this->closeStaticResource(*server, response.getResourceFd());
            }
            this->fdSet(client_fd, FdSet::WRITE);
            this->monitorTimeOutOff(pipe_fd);
        }
    }
    else
        this->monitorTimeOutOn(pipe_fd);
}

bool
ServerManager::isUnresponsiveFd(int fd)
{
    if (this->fdIsOriginSet(fd, FdSet::READ) &&
        this->fdIsOriginSet(fd, FdSet::READ) != this->fdIsCopySet(fd, FdSet::READ))
        return (true);
    if (this->fdIsOriginSet(fd, FdSet::WRITE) &&
        this->fdIsOriginSet(fd, FdSet::WRITE) != this->fdIsCopySet(fd, FdSet::WRITE))
        return (true);
    return (false);
}

Server*
ServerManager::findLinkedServer(int client_fd)
{
    for (Server* server : this->_servers)
    {
        if (server->getServerSocket() == this->getFdTable()[client_fd].second)
            return (server);
    }
    return (nullptr);
}

void
ServerManager::closeUnresponsiveFd()
{
    for (int fd = 3; fd < this->getFdMax() + 1; fd++)
    {
        if (this->isUnresponsiveFd(fd))
        {
            const FdType& type = this->getFdType(fd);
            switch (type)
            {
            case FdType::CLIENT_SOCKET:
                closeUnresponsiveClient(fd);
                break;
            
            case FdType::PIPE:
                closeUnresponsiveCgi(fd);
                break;

            default:
                break;
            }
        }
        else
            this->monitorTimeOutOff(fd);
    }
}

void
ServerManager::closeCgiWritePipe(Server& server, int write_fd_to_cgi)
{
    int client_fd = this->getLinkedFdFromFdTable(write_fd_to_cgi);
    server.getResponse(client_fd).setWriteFdToCgi(DEFAULT_FD);
    this->fdClr(write_fd_to_cgi, FdSet::READ);
    this->fdClr(write_fd_to_cgi, FdSet::WRITE);
    this->setClosedFdOnFdTable(write_fd_to_cgi);
    close(write_fd_to_cgi);
    this->updateFdMax(write_fd_to_cgi);
    this->monitorTimeOutOff(write_fd_to_cgi);
    Log::closeFd(FdType::PIPE, write_fd_to_cgi);
}

void
ServerManager::closeCgiReadPipe(Server& server, int read_fd_from_cgi)
{
    int client_fd = this->getLinkedFdFromFdTable(read_fd_from_cgi);
    server.getResponse(client_fd).setReadFdFromCgi(DEFAULT_FD);
    this->fdClr(read_fd_from_cgi, FdSet::READ);
    this->fdClr(read_fd_from_cgi, FdSet::WRITE);
    this->setClosedFdOnFdTable(read_fd_from_cgi);
    close(read_fd_from_cgi);
    this->monitorTimeOutOff(read_fd_from_cgi);
    this->updateFdMax(read_fd_from_cgi);
    this->monitorTimeOutOff(read_fd_from_cgi);
    Log::closeFd(FdType::PIPE, read_fd_from_cgi);
}

void
ServerManager::closeStaticResource(Server& server, int resource_fd)
{
    int client_fd = this->getLinkedFdFromFdTable(resource_fd);
    server.getResponse(client_fd).setResourceFd(DEFAULT_FD);
    this->fdClr(resource_fd, FdSet::READ);
    this->fdClr(resource_fd, FdSet::WRITE);
    this->setClosedFdOnFdTable(resource_fd);
    close(resource_fd);
    this->monitorTimeOutOff(resource_fd);
    this->updateFdMax(resource_fd);
    this->monitorTimeOutOff(resource_fd);
    Log::closeFd(FdType::RESOURCE, resource_fd);
}

void
ServerManager::clearServers()
{
    int status;

    for (Server* server : this->_servers)
    {
        for (int fd = 3; fd < this->getFdMax() + 1; fd++)
        {
            if (this->getFdTable()[fd].first == FdType::CLIENT_SOCKET)
            {
                kill(server->getResponse(fd).getCgiPid(), SIGKILL);
                waitpid(server->getResponse(fd).getCgiPid(), &status, WNOHANG);
            }
            close(fd);
            std::cout << "close fd: " << fd << std::endl;
        }
        delete server;
    }
}

void
ServerManager::exitServers(int signo)
{
    if (signo == SIGINT)
    {
        for (size_t i = 0; i < g_child_process_ids.size(); i++)
            kill(g_child_process_ids[i], SIGINT);
        exit(EXIT_SUCCESS);
    }
}
