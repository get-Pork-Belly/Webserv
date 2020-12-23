#ifndef SERVERMANAGER_HPP
# define SERVERMANAGER_HPP

# include <sys/types.h>
# include <sys/stat.h>
# include <sys/socket.h>
# include <sys/select.h>
# include <sys/time.h>
# include <errno.h>
# include <fcntl.h>
# include <unistd.h>
# include <map>
# include <exception>
# include <iostream>
# include <string>
# include <vector>
# include "types.hpp"
# include "ServerGenerator.hpp"
# include <signal.h>

const int DEFAULT_CLIENT_TIME_OUT_SECOND = 5;
const int DEFAULT_CGI_TIME_OUT_SECOND = 60;
const int DEFAULT_PID = 0;

class Server;

using MonitorStatus=bool;

class ServerManager
{
private:
    /* Canonical but not implements */
    ServerManager();
    ServerManager(const ServerManager& other);

private:
    const char*	_config_file_path;
    std::vector<Server *> _servers;
    fd_set _readfds;
    fd_set _writefds;
    fd_set _exceptfds;
    fd_set _copy_readfds;
    fd_set _copy_writefds;
    fd_set _copy_exceptfds;
    std::string _port;
    std::vector<std::pair<FdType, int>> _fd_table;
    int _fd;
    int _fd_max;
    std::vector<std::pair<MonitorStatus, timeval>> _last_update_time_of_fd;
    std::map<std::string, std::string> _plugins;
    int _client_timeout_second;
    int _cgi_timeout_second;

public:
    /* Constructor */
    ServerManager(const char *config_path);
    ServerManager& operator=(const ServerManager& rhs);
    /* Destructor */
    virtual ~ServerManager();
    /* Overload */
    /* Getter */
    const char *getConfigFilePath() const;
    int getFdMax() const;
    const std::vector<std::pair<FdType, int> >& getFdTable() const;
    FdType getFdType(int fd) const;
    int getLinkedFdFromFdTable(int fd) const;
    const std::map<std::string, std::string>& getPlugins() const;
    int getClientTimeoutSecond() const;
    int getCgiTimeoutSecond() const;
    /* Setter */
    void setFdMax(int fd);
    void setServerSocketOnFdTable(int fd);
    void setClientSocketOnFdTable(int fd, int server_socket);
    void setResourceOnFdTable(int fd, int client_socket);
    void setCgiPipeOnFdTable(int fd, int client_socket);
    void setClosedFdOnFdTable(int fd);
    void setLastUpdateTimeOfFd(int client_fd, MonitorStatus check, timeval* time);
    void setPlugins(std::map<std::string, std::string>& http_config);
    /* Exception */
    /* Util */
    bool fdIsCopySet(int fd, FdSet type);
    bool fdIsOriginSet(int fd, FdSet type);
    void fdClr(int fd, FdSet type);
    void fdSet(int fd, FdSet type);

    void updateFdMax(int fd);

    /* Manage Server functions */
    void initServers();
    bool runServers();
    void closeUnresponsiveFd();
    void closeUnresponsiveClient(int client_fd);
    void closeUnresponsiveCgi(int pipe_fd);
    void closeCgiWritePipe(Server& server, int pipe_fd);
    void closeCgiReadPipe(Server& server, int pipe_fd);
    void closeStaticResource(Server& server, int resource_fd);

    static void exitServers(int signo);
    void clearServers();

    bool isFdTimeOut(int fd);
    void monitorTimeOutOff(int fd);
    void monitorTimeOutOn(int fd);
    bool isMonitorTimeOutOn(int fd);

    bool isUnresponsiveFd(int fd);
    bool isCgiProcessTerminated(int clinet_fd) const;
    void terminateCgiProcess(int client_fd);
    Server* findLinkedServer(int client_fd);

    bool isPluginOn(const std::string& plugin_name) const;
};

#endif
