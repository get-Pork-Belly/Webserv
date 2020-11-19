#ifndef LOG_HPP
# define LOG_HPP

# include <iostream>
# include <fcntl.h>
# include <errno.h>
# include "Server.hpp"
# include "utils.hpp"

class ServerManager;

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
    static void openFd(Server& server, int client_socket, const FdType& type, int fd);
    static void closeFd(Server& server, int client_socket, const FdType& type, int fd);
    static void getRequest(Server& server, int fd);
    static void timeLog(int fd);
    // error
    static void error(const std::string& message);
    // static void notAllowedMethod(Server& server);
    // static void fileNotfound();

    // trace
    static void trace(const std::string& message, int log_level);
    static void printTimeDiff(timeval from, int log_level);

    static std::string fdTypeToString(const FdType& type);
    static std::string resTypeToString(const ResType& type);
    static std::string sendProgressToString(const SendProgress& progress);
    static void printLocationConfig(const std::map<std::string, location_info>& loc_config);
    static void printLocationInfo(const location_info& loc_info);

    static void printFdCopySets(ServerManager& server_manager);
    static void printFdSets(ServerManager& server_manager);
};

#endif

