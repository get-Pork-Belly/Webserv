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
# include "types.hpp"
# include "utils.hpp"
# include "ServerGenerator.hpp"

class Server;

class ServerManager
{
private:
    /* Canonical but not implements */
    ServerManager();
    ServerManager(const ServerManager& other);
    ServerManager& operator=(const ServerManager& rhs);

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
    int _fd;
    int _fd_max;
    std::map<int, std::string> _status_code_msg;

public:
    /* Constructor */
    ServerManager(const char *config_path);
    /* Destructor */
    virtual ~ServerManager();
    /* Overload */
    /* Getter */
    const char *getConfigFilePath() const;
    const int getFdMax() const;
    /* Setter */
    /* Exception */
    /* Util */

    /* Manage Server functions */
    void initServers();
    bool runServers();
    //TODO 구현 필요
    // void exitServers();
};

#endif