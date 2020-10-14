#ifndef SERVERMANAGER_HPP
# define SERVERMANAGER_HPP

# include <sys/types.h>
# include <sys/stat.h>
# include <sys/socket.h>
# include <sys/select.h>
# include <sys/time.h>
# include <errno.h>
# include <map>
# include <exception>
# include <fcntl.h>
# include <unistd.h>
# include <iostream>
# include <string>
# include "types.hpp"
# include "utils.hpp"

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
    struct GlobalConfig _global_config;

public:
    /* Constructor */
    ServerManager(const char *config_path);
    /* Destructor */
    virtual ~ServerManager();
    /* Overload */
    /* Getter */
    const struct GlobalConfig &getGlobalConfig();
    /* Setter */
    /* Exception */
    /* Util */

    /* Manage Server functions */
    void initServers();
    void makeServer(struct ServerConfig);
    bool runServers();
    void exitServers();
};

#endif
