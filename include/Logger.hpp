#ifndef LOGGER_HPP
# define LOGGER_HPP

# include <iostream>
# include <fcntl.h>
# include <errno.h>
# include <cstring>
# include <sys/time.h>
# include <unistd.h>
# include "Server.hpp"
# include "utils.hpp"

class Logger
{
private:
    Logger();
    Logger(const Logger& other);
    Logger& operator=(const Logger& rhs);

public:
    static int access_fd;
    static int error_fd;
    
    // access
    static void serverWasCreated(Server& server);
    static void serverHasNewClient(Server& server, int client_fd);
    static void serverCloseClient(Server& server, int client_fd);
    static void serverGetRequest(Server& server, int fd);
    // error
    // static void notAllowedMethod(Server& server);
    // static void fileNotfound();

};

#endif

