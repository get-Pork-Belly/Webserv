#include "Log.hpp"
#include "ServerManager.hpp"

/*============================================================================*/
/****************************  Static variables  ******************************/
/*============================================================================*/

/*============================================================================*/
/******************************  Constructor  *********************************/
/*============================================================================*/

int
Log::log_fd = -1;

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

bool
Log::isLogPluginOn()
{
    if (Log::log_fd == -1)
        return (false);
    return (true);
}

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
    int res;
    res = write(fd, static_cast<void *>(buf), ft::strlen(buf));
    if (res == 0)
        std::cout << "log failed! but keep server going" << std::endl;
    else if (res < 0)
        std::cout << "log failed! but keep server going" << std::endl;
}

void
Log::printTimeSec(timeval& tv)
{
    struct tm time;
    char buf[64];
    ft::memset(buf, 0, sizeof(buf));
    strptime(std::to_string(tv.tv_sec).c_str(), "%s", &time);
    strftime(buf, sizeof(buf), "[%d/%b/%Y/%X %Z] ", &time);
    int res;
    res = write(Log::log_fd, static_cast<void *>(buf), ft::strlen(buf));
    if (res == 0)
        std::cout << "log failed! but keep server going" << std::endl;
    else if (res < 0)
        std::cout << "log failed! but keep server going" << std::endl;
}

void
Log::serverIsCreated(Server& server)
{
    if (Log::isLogPluginOn() == false)
        return ;

    std::string line;
    int server_fd = server.getServerSocket();

    line = ("SERVER(" + std::to_string(server_fd) + ")" + "[" +
            server.getHost() + "] HAS CREATED\n");
    Log::timeLog(Log::log_fd);
    int res;
    res = write(Log::log_fd, line.c_str(), line.length());
    if (res == 0)
        std::cout << "log failed! but keep server going" << std::endl;
    else if (res < 0)
        std::cout << "log failed! but keep server going" << std::endl;
}

void
Log::newClient(Server& server, int client_fd)
{
    if (Log::isLogPluginOn() == false)
        return ;

    std::string line;
    int server_fd = server.getServerSocket();

    line = ("SERVER(" + std::to_string(server_fd) +  ")[" +
            server.getHost() + "] HAS NEW CLIENT: " +
            std::to_string(client_fd) + "\n");
    Log::timeLog(Log::log_fd);
    int res;
    res = write(Log::log_fd, line.c_str(), line.length());
    if (res == 0)
        std::cout << "log failed! but keep server going" << std::endl;
    else if (res < 0)
        std::cout << "log failed! but keep server going" << std::endl;
}

void
Log::closeClient(Server& server, int client_fd)
{
    if (Log::isLogPluginOn() == false)
        return ;

    std::string line;
    int server_fd = server.getServerSocket();

    line = ("SERVER(" + std::to_string(server_fd) +
            ") BYBY CLIENT(" + std::to_string(client_fd)
           + ")\n");
    Log::timeLog(Log::log_fd);
    int res;
    res = write(Log::log_fd, line.c_str(), line.length());
    if (res == 0)
        std::cout << "log failed! but keep server going" << std::endl;
    else if (res < 0)
        std::cout << "log failed! but keep server going" << std::endl;
}

void
Log::openFd(Server& server, int client_socket, const FdType& type, int fd)
{
    if (Log::isLogPluginOn() == false)
        return ;

    int server_fd = server.getServerSocket();
    std::string line;
    line = "SERVER(" + std::to_string(server_fd) + ") OPEN " 
                    + fdTypeToString(type) + "(" + std::to_string(fd) 
                    + ") which requested by CLIENT(" 
                    + std::to_string(client_socket) + ")\n";

    Log::timeLog(Log::log_fd);
    int res;
    res = write(Log::log_fd, line.c_str(), line.length());
    if (res == 0)
        std::cout << "log failed! but keep server going" << std::endl;
    else if (res < 0)
        std::cout << "log failed! but keep server going" << std::endl;
}

void
Log::closeFd(Server& server, int client_socket, const FdType& type, int fd)
{
    if (Log::isLogPluginOn() == false)
        return ;

    int server_fd = server.getServerSocket();

    std::string line;
    line = "SERVER(" + std::to_string(server_fd) + ") CLOSE " 
                    + fdTypeToString(type) + "(" + std::to_string(fd) 
                    + ") which requested by CLIENT(" 
                    + std::to_string(client_socket) + ")\n";

    Log::timeLog(Log::log_fd);
    int res;
    res = write(Log::log_fd, line.c_str(), line.length());
    if (res == 0)
        std::cout << "log failed! but keep server going" << std::endl;
    else if (res < 0)
        std::cout << "log failed! but keep server going" << std::endl;
}

void
Log::closeFd(const FdType& type, int fd)
{
    if (Log::isLogPluginOn() == false)
        return ;

    std::string line;
    line = "SEVER CLOSES " 
                    + fdTypeToString(type) + "(" + std::to_string(fd) 
                    + ") which requested by CLIENT\n";

    Log::timeLog(Log::log_fd);
    int res;
    res = write(Log::log_fd, line.c_str(), line.length());
    if (res == 0)
        std::cout << "log failed! but keep server going" << std::endl;
    else if (res < 0)
        std::cout << "log failed! but keep server going" << std::endl;
}

void
Log::getRequest(Server& server, int client_fd)
{
    if (Log::isLogPluginOn() == false)
        return ;

    std::string line;
    int server_fd = server.getServerSocket();
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
    Log::timeLog(Log::log_fd);
    int res;
    res = write(Log::log_fd, line.c_str(), line.length());
    if (res == 0)
        std::cout << "log failed! but keep server going" << std::endl;
    else if (res < 0)
        std::cout << "log failed! but keep server going" << std::endl;
}

void
Log::error(const std::string& error)
{
    if (Log::isLogPluginOn() == false)
        return ;

    Log::timeLog(Log::log_fd);
    int res;
    res = write(Log::log_fd, error.c_str(), error.length());
    if (res == 0)
        std::cout << "log failed! but keep server going" << std::endl;
    else if (res < 0)
        std::cout << "log failed! but keep server going" << std::endl;
}

void
Log::printTimeDiff(timeval from, int log_level)
{
    if (DEBUG < log_level)
        return ;
    timeval t;
    gettimeofday(&t, NULL);
    std::string diff = std::to_string((t.tv_sec - from.tv_sec) * 1000000 + (t.tv_usec - from.tv_usec));
    diff.push_back(' ');

    int res;
    res = write(Log::log_fd, diff.c_str(), diff.length());
    if (res == 0)
        std::cout << "log failed! but keep server going" << std::endl;
    else if (res < 0)
        std::cout << "log failed! but keep server going" << std::endl;
}

void
Log::trace(const std::string& trace, int log_level)
{
    if (DEBUG < log_level)
        return ;
    std::string line;
    int res;
    res = write(Log::log_fd, trace.c_str(), trace.length());
    if (res == 0)
        std::cout << "log failed! but keep server going" << std::endl;
    else if (res < 0)
        std::cout << "log failed! but keep server going" << std::endl;
    res = write(Log::log_fd, "\n", 1);
    if (res == 0)
        std::cout << "log failed! but keep server going" << std::endl;
    else if (res < 0)
        std::cout << "log failed! but keep server going" << std::endl;
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
        return ("Cgi");

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
