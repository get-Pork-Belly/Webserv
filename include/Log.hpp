#ifndef LOG_HPP
# define LOG_HPP

# include <iostream>
# include <fcntl.h>
# include <errno.h>
# include "Server.hpp"
# include "utils.hpp"

class Log
{
private:
    Log();
    Log(const Log& other);
    Log& operator=(const Log& rhs);

public:
    static int access_fd;
    static int error_fd;
    
    // access
    static void serverIsCreated(Server& server);
    static void newClient(Server& server, int client_fd);
    static void closeClient(Server& server, int client_fd);
    static void getRequest(Server& server, int fd);
    static void timeLog(int fd);
    // error
    static void error(const std::string& message);
    // static void notAllowedMethod(Server& server);
    // static void fileNotfound();

};

#endif

