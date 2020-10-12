#ifndef SERVERMANAGER_HPP
# define SERVERMANAGER_HPP

# include <sys/types.h>
# include <sys/socket.h>
# include <map>
# include "types.hpp"

class ServerManager
{
private:
    /* Canonical but not implements */
    ServerManager();
    ServerManager(const ServerManager& other);
    ServerManager& operator=(const ServerManager& rhs);

private:
    std::string _config_file_path;
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
    /* Setter */
    /* Exception */
    /* Util */

    /* Manage Server functions */
    void initServers();
    void makeServer(struct s_config);
    bool runServers();
    void exitServers();
};

#endif