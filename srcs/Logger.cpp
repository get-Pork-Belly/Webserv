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
Logger::serverWasCreated(Server& server)
{
    if (DEBUG == 0)
        return ;
    int server_fd = server.getServerSocket();
    char time_buffer[30];
    std::string line;
    time_t raw_time;
    struct tm* time_info;

    time(&raw_time);
    time_info = localtime(&raw_time);
    ft::memset(reinterpret_cast<void *>(time_buffer), 0, 30);
    strftime(time_buffer, 30,
            "[%d/%b/%Y/%X GMT] ", time_info);

    line = ("SERVER(\033[1;31;40m" + std::to_string(server_fd) +
            "\033[0m) HAS CREATED\n");

    write(Logger::access_fd, time_buffer, strlen(time_buffer));
    write(Logger::access_fd, line.c_str(), line.length());
    if (STDOUT == 1)
    {
        write(1, time_buffer, strlen(time_buffer));
        write(1, line.c_str(), line.length());
    }
}

void
Logger::serverHasNewClient(Server& server, int client_fd)
{
    if (DEBUG == 0)
        return ;
    int server_fd = server.getServerSocket();
    char time_buffer[30];
    std::string line;
    time_t raw_time;
    struct tm* time_info;

    time(&raw_time);
    time_info = localtime(&raw_time);
    ft::memset(reinterpret_cast<void *>(time_buffer), 0, 30);
    strftime(time_buffer, 30,
            "[%d/%b/%Y/%X GMT] ", time_info);

    line = ("SERVER(\033[1;31;40m" + std::to_string(server_fd) +
            "\033[0m) HAS NEW CLIENT: \033[1;31;40m" + std::to_string(client_fd)
            + "\033[0m\n");

    write(Logger::access_fd, time_buffer, strlen(time_buffer));
    write(Logger::access_fd, line.c_str(), line.length());
    if (STDOUT == 1)
    {
        write(1, time_buffer, strlen(time_buffer));
        write(1, line.c_str(), line.length());
    }
}

void
Logger::serverCloseClient(Server& server, int client_fd)
{
    if (DEBUG == 0)
        return ;
    int server_fd = server.getServerSocket();
    char time_buffer[30];
    std::string line;
    time_t raw_time;
    struct tm* time_info;

    time_info = localtime(&raw_time);
    time(&raw_time);
    ft::memset(reinterpret_cast<void *>(time_buffer), 0, 30);
    strftime(time_buffer, 30,
            "[%d/%b/%Y/%X GMT] ", time_info);

    line = ("SERVER(\033[1;31;40m" + std::to_string(server_fd) +
            "\033[0m) BYBY CLIENT(\033[1;31;40m" + std::to_string(client_fd)
           + "\033[0m)\n");
    write(Logger::access_fd, time_buffer, strlen(time_buffer));
    write(Logger::access_fd, line.c_str(), line.length());
    if (STDOUT == 1)
    {
        write(1, time_buffer, strlen(time_buffer));
        write(1, line.c_str(), line.length());
    }
}

void
Logger::serverGetRequest(Server& server, int client_fd)
{
    if (DEBUG == 0)
        return ;
    char time_buffer[30];
    time_t raw_time;
    struct tm* time_info;
    std::string line;
    int server_fd = server.getServerSocket();
    std::string method = server.getRequest(client_fd).getRequestMethod();
    std::string uri = server.getRequest(client_fd).getRequestUri();

    time(&raw_time);
    time_info = localtime(&raw_time);
    ft::memset(reinterpret_cast<void *>(time_buffer), 0, 30);
    strftime(time_buffer, 30,
            "[%d/%b/%Y/%X GMT] ", time_info);

    line = ("SERVER(\033[1;31;40m" + std::to_string(server_fd) +
            "\033[0m) GOT REQUEST FROM: CLIENT(\033[1;31;40m" +
            std::to_string(client_fd) + "\033[0m)" + " \"" + method + " " +
            uri + "\"\n");
    write(Logger::access_fd, time_buffer, strlen(time_buffer));
    write(Logger::access_fd, line.c_str(), line.length());
    if (STDOUT == 1)
    {
        write(1, time_buffer, strlen(time_buffer));
        write(1, line.c_str(), line.length());
    }
}
