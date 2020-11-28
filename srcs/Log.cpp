#include "Log.hpp"
#include "ServerManager.hpp"

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
Log::printTimeSec(timeval& tv)
{
    struct tm time;
    char buf[64];
    ft::memset(buf, 0, sizeof(buf));
    strptime(std::to_string(tv.tv_sec).c_str(), "%s", &time);
    strftime(buf, sizeof(buf), "[%d/%b/%Y/%X %Z] ", &time);
    write(1, static_cast<void *>(buf), ft::strlen(buf));
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
Log::openFd(Server& server, int client_socket, const FdType& type, int fd)
{
    if (DEBUG == 0)
        return ;

    int server_fd = server.getServerSocket();
    int log_print_fd = (STDOUT == 1) ? 1 : Log::access_fd;

    std::string line;
    line = "SERVER(" + std::to_string(server_fd) + ") OPEN " 
                    + fdTypeToString(type) + "(" + std::to_string(fd) 
                    + ") which requested by CLIENT(" 
                    + std::to_string(client_socket) + ")\n";

    Log::timeLog(log_print_fd);
    write(log_print_fd, line.c_str(), line.length());
}

void
Log::closeFd(Server& server, int client_socket, const FdType& type, int fd)
{
    if (DEBUG == 0)
        return ;

    int server_fd = server.getServerSocket();
    int log_print_fd = (STDOUT == 1) ? 1 : Log::access_fd;

    std::string line;
    line = "SERVER(" + std::to_string(server_fd) + ") CLOSE " 
                    + fdTypeToString(type) + "(" + std::to_string(fd) 
                    + ") which requested by CLIENT(" 
                    + std::to_string(client_socket) + ")\n";

    Log::timeLog(log_print_fd);
    write(log_print_fd, line.c_str(), line.length());
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


void
Log::printTimeDiff(timeval from, int log_level)
{
    timeval t;
    gettimeofday(&t, NULL);
    std::string diff = std::to_string((t.tv_sec - from.tv_sec) * 1000000 + (t.tv_usec - from.tv_usec));
    diff.push_back(' ');

    if (DEBUG < log_level)
        return ;
    int fd = (STDOUT == 1) ? 1 : Log::access_fd;
    write(fd, diff.c_str(), diff.length());
}

void
Log::trace(const std::string& trace, int log_level)
{
    if (DEBUG < log_level)
        return ;

    std::string line;
    int fd = (STDOUT == 1) ? 1 : Log::access_fd;

    // Log::timeLog(fd);
    write(fd, trace.c_str(), trace.length());
    write(fd, "\n", 1);
}

std::string
Log::fdTypeToString(const FdType& type)
{
    switch (type)
    {
    case FdType::SERVER_SOCKET:
        return ("SERVER");

    case FdType::CLIENT_SOCKET:
        return ("CLIENT");

    case FdType::RESOURCE:
        return ("STATIC_RESOURCE");

    case FdType::PIPE:
        return ("PIPE");

    case FdType::CLOSED:
        return ("CLOSED");

    default:
        break;
    }
}

std::string
Log::resTypeToString(const ResType& type)
{
    switch (type)
    {
    case ResType::STATIC_RESOURCE:
        return ("STATIC_RESOURCE");

    case ResType::CGI:
        return ("CGI");

    case ResType::AUTO_INDEX:
        return ("AUTO_INDEX");

    case ResType::ERROR_HTML:
        return ("ERROR_HTML");

    default:
        return ("NOT YET REGISTED IN resTypeToString");
        break;
    }
}

std::string
Log::recvRequestToString(const RecvRequest& req_request_progress)
{
    switch (req_request_progress)
    {
    case RecvRequest::REQUEST_LINE:
        return ("REQUEST_LINE");

    case RecvRequest::COMPLETE:
        return ("COMPLETE");

    case RecvRequest::NORMAL_BODY:
        return ("NORMAL_BODY");

    case RecvRequest::CHUNKED_BODY:
        return ("CHUNKED_BODY");

    default:
        return ("NOT YET REGISTED IN recvRequestToString");
        break;
    }
}

std::string
Log::parseProgressToString(const ParseProgress& progress)
{
    switch (progress)
    {
    case ParseProgress::DEFAULT:
        return ("DEFAULT");

    case ParseProgress::CHUNK_START:
        return ("CHUNK_START");

    case ParseProgress::CHUNK_PROGRESS:
        return ("CHUNK_PROGRESS");

    case ParseProgress::FINISH:
        return ("FINISH");

    default:
        return ("NOT YET REGISTED IN sendProgress");
        break;
    }
}



void
Log::printLocationConfig(const std::map<std::string, location_info>& loc_config)
{
    if (DEBUG != 2)
        return ;

    for(auto& kv: loc_config)
    {
        std::cout<<"|==============================================="<<std::endl;
        std::cout<<"| Route: "<<kv.first<<std::endl;
        printLocationInfo(kv.second);
    }
        std::cout<<"|==============================================="<<std::endl;
}

void
Log::printLocationInfo(const location_info& loc_info)
{
    if (DEBUG != 2)
        return ;

    for(auto& kv : loc_info)
        std::cout<<"| "<<kv.first<<" : "<<kv.second<<std::endl;
}

void
Log::printFdCopySets(ServerManager& server_manager, int width)
{
    int max_fd = server_manager.getFdMax() + 1;
    bool is_set;

    std::cout<<"READ FD COPY SET"<<std::endl;
    int start_idx = 0;
    while(start_idx < max_fd)
    {
        for (int i = start_idx; i < std::min(start_idx + width, max_fd); i++)
            if (server_manager.getFdType(i) == FdType::RESOURCE)
                std::cout<<std::right<<"|"<<std::setw(7)<< " STATIC ";
            else
                std::cout<<std::right<<"|"<<std::setw(7)<<Log::fdTypeToString(server_manager.getFdType(i))<<" ";
        std::cout<<"|"<<std::endl;
        for (int i = start_idx; i < std::min(start_idx + width, max_fd); i++)
            std::cout<<"| fd "<<std::setw(3)<<std::right<<i<<" ";
        std::cout<<"|"<<std::endl;

        for (int i = start_idx; i < std::min(start_idx + width, max_fd); i++)
        {
            is_set = server_manager.fdIsCopySet(i, FdSet::READ);
            std::string color;
            if (is_set)
            {
                if (server_manager.getFdType(i) == FdType::CLOSED)
                    color = "\033[46;1;97m";
                else
                    color = "\033[41;1;97m";
                std::cout<<"| ";
                std::cout<<color<<std::setw(6)<<is_set<<"\033[0m";
                std::cout<<" ";
                // std::cout<<"| ";
                // std::cout<<"\033[41;1;97m"<<std::setw(6)<<is_set<<"\033[0m";
                // std::cout<<" ";
            }
            else
                std::cout<<"| "<<std::setw(6)<<is_set<<" ";
        }
        std::cout<<"|"<<std::endl;
        start_idx += width;
    }

    std::cout<<"WRITE FD COPY SET"<<std::endl;
    start_idx = 0;
    while(start_idx < max_fd)
    {
        for (int i = start_idx; i < std::min(start_idx + width, max_fd); i++)
            if (server_manager.getFdType(i) == FdType::RESOURCE)
                std::cout<<std::right<<"|"<<std::setw(7)<< " STATIC ";
            else
                std::cout<<std::right<<"|"<<std::setw(7)<<Log::fdTypeToString(server_manager.getFdType(i))<<" ";
        std::cout<<"|"<<std::endl;
        for (int i = start_idx; i < std::min(start_idx + width, max_fd); i++)
            std::cout<<"| fd "<<std::setw(3)<<std::right<<i<<" ";
        std::cout<<"|"<<std::endl;

        for (int i = start_idx; i < std::min(start_idx + width, max_fd); i++)
        {
            is_set = server_manager.fdIsCopySet(i, FdSet::WRITE);
            std::string color;
            if (is_set)
            {
                if (server_manager.getFdType(i) == FdType::CLOSED)
                    color = "\033[46;1;97m";
                else
                    color = "\033[41;1;97m";
                std::cout<<"| ";
                std::cout<<color<<std::setw(6)<<is_set<<"\033[0m";
                std::cout<<" ";
                // std::cout<<"| ";
                // std::cout<<"\033[41;1;97m"<<std::setw(6)<<is_set<<"\033[0m";
                // std::cout<<" ";
            }
            else
                std::cout<<"| "<<std::setw(6)<<is_set<<" ";
        }
        std::cout<<"|"<<std::endl;
        start_idx += width;
    }
}

void
Log::printFdSets(ServerManager& server_manager, int width)
{
    int max_fd = server_manager.getFdMax() + 1;
    bool is_set;

    std::cout<<"READ FD ORIGIN SET"<<std::endl;
    int start_idx = 0;
    while(start_idx < max_fd)
    {
        for (int i = start_idx; i < std::min(start_idx + width, max_fd); i++)
        {
            if (server_manager.getFdType(i) == FdType::RESOURCE)
                std::cout<<std::right<<"|"<<std::setw(7)<< " STATIC ";
            else
                std::cout<<std::right<<"|"<<std::setw(7)<<Log::fdTypeToString(server_manager.getFdType(i))<<" ";
        }
        std::cout<<"|"<<std::endl;
        for (int i = start_idx; i < std::min(start_idx + width, max_fd); i++)
            std::cout<<"| fd "<<std::setw(3)<<std::right<<i<<" ";
        std::cout<<"|"<<std::endl;

        for (int i = start_idx; i < std::min(start_idx + width, max_fd); i++)
        {
            is_set = server_manager.fdIsOriginSet(i, FdSet::READ);
            std::string color;
            if (is_set)
            {
                if (server_manager.getFdType(i) == FdType::CLOSED)
                    color = "\033[46;1;97m";
                else
                    color = "\033[41;1;97m";
                std::cout<<"| ";
                std::cout<<color<<std::setw(6)<<is_set<<"\033[0m";
                std::cout<<" ";
                // std::cout<<"| ";
                // std::cout<<"\033[41;1;97m"<<std::setw(6)<<is_set<<"\033[0m";
                // std::cout<<" ";
            }
            else
                std::cout<<"| "<<std::setw(6)<<is_set<<" ";
        }
        std::cout<<"|"<<std::endl;
        start_idx += width;
    }

    std::cout<<"WRITE FD ORIGIN SET"<<std::endl;
    start_idx = 0;
    while(start_idx < max_fd)
    {
        for (int i = start_idx; i < std::min(start_idx + width, max_fd); i++)
            if (server_manager.getFdType(i) == FdType::RESOURCE)
                std::cout<<std::right<<"|"<<std::setw(7)<< " STATIC ";
            else
                std::cout<<std::right<<"|"<<std::setw(7)<<Log::fdTypeToString(server_manager.getFdType(i))<<" ";
        std::cout<<"|"<<std::endl;
        for (int i = start_idx; i < std::min(start_idx + width, max_fd); i++)
            std::cout<<"| fd "<<std::setw(3)<<std::right<<i<<" ";
        std::cout<<"|"<<std::endl;

        for (int i = start_idx; i < std::min(start_idx + width, max_fd); i++)
        {
            is_set = server_manager.fdIsOriginSet(i, FdSet::WRITE);
            std::string color;
            // std::string purple = "\033[41;35m";
            if (is_set)
            {
                if (server_manager.getFdType(i) == FdType::CLOSED)
                    color = "\033[46;1;97m";
                else
                    color = "\033[41;1;97m";
                std::cout<<"| ";
                std::cout<<color<<std::setw(6)<<is_set<<"\033[0m";
                std::cout<<" ";
            }
            else
                std::cout<<"| "<<std::setw(6)<<is_set<<" ";
        }
        std::cout<<"|"<<std::endl;
        start_idx += width;
    }
}
