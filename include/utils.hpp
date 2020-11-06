#ifndef UTILS_HPP
# define UTILS_HPP

# include <vector>
# include <string>
# include <sys/socket.h>
# include <sys/types.h>
# include <sys/time.h>
# include <cstring>
# include <unistd.h>
# include <map>

namespace ft
{
    std::vector<std::string> split(std::string str, std::string const &token);
    std::string	ltrim(const std::string& str, char const *token = "\t\n ");
    std::string	rtrim(const std::string& str, char const *token = "\t\n ");

    void fdZero(fd_set *fds);
    void fdSet(int fd, fd_set *fds);
    bool fdIsSet(int fd, fd_set *fds);
    void fdClr(int fd, fd_set *fds);
  
    unsigned long hToNL(unsigned long hostlong);
    unsigned short hToNS(unsigned short hostshort);
    unsigned long nToHL(unsigned long hostlong);
    unsigned short nToHS(unsigned short hostshort);

    bool substr(std::string &line, std::string &lines, const std::string &delim);
    void* memset(void* b, int c, size_t len);

    int stoiHex(const std::string& str);

    int getTimeDiffBetweenGMT(char *time_zone);
    std::string getCurrentDateTime();
    size_t strlen(const char* str);
    char* strdup(const std::string& s);
    std::string inetNtoA(const in_addr_t& client_address);
    void doubleFree(char** target);
}

#endif
