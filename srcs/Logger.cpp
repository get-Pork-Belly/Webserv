#include "Logger.hpp"

/*============================================================================*/
/****************************  Static variables  ******************************/
/*============================================================================*/

/*============================================================================*/
/******************************  Constructor  *********************************/
/*============================================================================*/

int
Logger::access_fd = open("./log/access_log",
        O_CREAT | O_APPEND | O_WRONLY, 0644);

int
Logger::error_fd = open("./log/error_log",
        O_CREAT | O_APPEND | O_WRONLY, 0644);

/*============================================================================*/
/******************************  Destructor  **********************************/
/*============================================================================*/

/*============================================================================*/
/********************************  Getter  ************************************/
/*============================================================================*/

/*============================================================================*/
/********************************  Setter  ************************************/
/*============================================================================*/

/*============================================================================*/
/******************************  Exception  ***********************************/
/*============================================================================*/

/*============================================================================*/
/*********************************  Util  *************************************/
/*============================================================================*/

void
Logger::timeLog(int fd)
{
    struct tm time;
    struct timeval tv;
    char buf[64];
    std::string date;

    gettimeofday(&tv, NULL);
    ft::memset(buf, 0, sizeof(buf));
    strptime(std::to_string(tv.tv_sec).c_str(), "%s", &time);
    strftime(buf, sizeof(buf), "[%d/%b/%Y/%X GMT] ", &time);
    write(fd, static_cast<void *>(buf), sizeof(buf));
}

void
Logger::serverWasCreated(Server& server)
{
    if (DEBUG == 0)
        return ;

    std::string line;
    int server_fd = server.getServerSocket();
    int fd = (STDOUT == 1) ? 1 : Logger::access_fd;

    line = ("SERVER(\033[1;31;40m" + std::to_string(server_fd) +
            "\033[0m) HAS CREATED\n");
    Logger::timeLog(fd);
    write(fd, line.c_str(), line.length());
}

void
Logger::serverHasNewClient(Server& server, int client_fd)
{
    if (DEBUG == 0)
        return ;

    std::string line;
    int server_fd = server.getServerSocket();
    int fd = (STDOUT == 1) ? 1 : Logger::access_fd;

    line = ("SERVER(\033[1;31;40m" + std::to_string(server_fd) +
            "\033[0m) HAS NEW CLIENT: \033[1;31;40m" +
            std::to_string(client_fd) + "\033[0m\n");
    Logger::timeLog(fd);
    write(fd, line.c_str(), line.length());
}

void
Logger::serverCloseClient(Server& server, int client_fd)
{
    if (DEBUG == 0)
        return ;

    std::string line;
    int server_fd = server.getServerSocket();
    int fd = (STDOUT == 1) ? 1 : Logger::access_fd;

    line = ("SERVER(\033[1;31;40m" + std::to_string(server_fd) +
            "\033[0m) BYBY CLIENT(\033[1;31;40m" + std::to_string(client_fd)
           + "\033[0m)\n");
    Logger::timeLog(fd);
    write(fd, line.c_str(), line.length());
}

void
Logger::serverGetRequest(Server& server, int client_fd)
{
    if (DEBUG == 0)
        return ;

    std::string line;
    int server_fd = server.getServerSocket();
    int fd = (STDOUT == 1) ? 1 : Logger::access_fd;
    std::string method = server.getRequest(client_fd).getRequestMethod();
    std::string uri = server.getRequest(client_fd).getRequestUri();
    std::string version = server.getRequest(client_fd).getRequestVersion();

    line = ("SERVER(\033[1;31;40m" + std::to_string(server_fd) +
            "\033[0m) GOT REQUEST FROM: CLIENT(\033[1;31;40m" +
            std::to_string(client_fd) + "\033[0m)" + " \"" + method + " " +
            uri + " "+ version + "\"\n");
    Logger::timeLog(fd);
    write(fd, line.c_str(), line.length());
}

void
Logger::error(const std::string& error)
{
    if (DEBUG == 0)
        return ;

    int fd = (STDOUT == 1) ? 1 : Logger::error_fd;
    write(fd, error.c_str(), error.length());
}
