#include "Log.hpp"

/*============================================================================*/
/****************************  Static variables  ******************************/
/*============================================================================*/

/*============================================================================*/
/******************************  Constructor  *********************************/
/*============================================================================*/

int
Log::access_fd = open("./log/access_log",
        O_CREAT | O_TRUNC | O_WRONLY, 0644);

int
Log::error_fd = open("./log/error_log",
        O_CREAT | O_TRUNC | O_WRONLY, 0644);

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
Log::timeLog(int fd)
{
    struct tm time;
    struct timeval tv;
    char buf[64];
    std::string date;

    gettimeofday(&tv, NULL);
    ft::memset(buf, 0, sizeof(buf));
    strptime(std::to_string(tv.tv_sec).c_str(), "%s", &time);
    strftime(buf, sizeof(buf), "[%d/%b/%Y/%X %Z] ", &time);
    write(fd, static_cast<void *>(buf), ft::strlen(buf));
}

void
Log::serverIsCreated(Server& server)
{
    if (DEBUG == 0)
        return ;

    std::string line;
    int server_fd = server.getServerSocket();
    int fd = (STDOUT == 1) ? 1 : Log::access_fd;

    line = ("SERVER(" + std::to_string(server_fd) +
            ") HAS CREATED\n");
    Log::timeLog(fd);
    write(fd, line.c_str(), line.length());
}

void
Log::newClient(Server& server, int client_fd)
{
    if (DEBUG == 0)
        return ;

    std::string line;
    int server_fd = server.getServerSocket();
    int fd = (STDOUT == 1) ? 1 : Log::access_fd;

    line = ("SERVER(" + std::to_string(server_fd) +
            ") HAS NEW CLIENT: " +
            std::to_string(client_fd) + "\n");
    Log::timeLog(fd);
    write(fd, line.c_str(), line.length());
}

void
Log::closeClient(Server& server, int client_fd)
{
    if (DEBUG == 0)
        return ;

    std::string line;
    int server_fd = server.getServerSocket();
    int fd = (STDOUT == 1) ? 1 : Log::access_fd;

    line = ("SERVER(" + std::to_string(server_fd) +
            ") BYBY CLIENT(" + std::to_string(client_fd)
           + ")\n");
    Log::timeLog(fd);
    write(fd, line.c_str(), line.length());
}

void
Log::getRequest(Server& server, int client_fd)
{
    if (DEBUG == 0)
        return ;

    std::string line;
    int server_fd = server.getServerSocket();
    int fd = (STDOUT == 1) ? 1 : Log::access_fd;
    std::string method = server.getRequest(client_fd).getMethod();
    std::string uri = server.getRequest(client_fd).getUri();
    std::string version = server.getRequest(client_fd).getVersion();

    if (!method.empty() && !uri.empty() && !version.empty())
    {
        line = ("SERVER(" + std::to_string(server_fd) +
                ") GOT REQUEST FROM: CLIENT(" +
                std::to_string(client_fd) + ")" + " \"" + method + " " +
                uri + " "+ version + "\"\n");
    }
    else
    {
        line = ("SERVER(" + std::to_string(server_fd) + ") READ CLIENT(" +
        std::to_string(client_fd) + ") BUFFER BUT EMPTY NOW\n"); 
    }
    Log::timeLog(fd);
    write(fd, line.c_str(), line.length());
}

void
Log::error(const std::string& error)
{
    if (DEBUG == 0)
        return ;

    int fd = (STDOUT == 1) ? 1 : Log::error_fd;
    Log::timeLog(fd);
    write(fd, error.c_str(), error.length());
}
