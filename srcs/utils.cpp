#include "utils.hpp"

namespace ft {

std::vector<std::string> split(std::string str, std::string const &token)
{
    std::vector<std::string> result;
    size_t index;
    std::string tmp;

    while (str.length())
    {
        index = str.find(token);
        if (index != std::string::npos)
        {
            tmp = str.substr(0, index);
            if (tmp.length())
                result.push_back(tmp);
            str = str.substr(index + token.length());
        }
        else
        {
            result.push_back(str);
            str = "";
        }
    }
    return (result);
}

std::string ltrim(const std::string& str, const char *token)
{
    size_t n = str.find_first_not_of(token);
    return (n == std::string::npos ? str : str.substr(n, str.length()));
}

std::string rtrim(const std::string& str, const char *token)
{
    size_t n = str.find_last_not_of(token);
    return (n == std::string::npos ? str : str.substr(0, n + 1));
}

void fdZero(fd_set *fds)
{
    for (int i = 0; i < 32; ++i)
        fds->fds_bits[i] = 0;
}

void fdSet(int fd, fd_set *fds)
{
    if (fd < 0 || fd >= 1024)
        return ;
    int mask = 1 <<((unsigned long)fd % (sizeof(__int32_t) * 8));
    fds->fds_bits[(unsigned long)fd/(sizeof(__int32_t) * 8)] |= mask;
}

bool fdIsset(int fd, fd_set *fds)
{
    if (fd < 0 || fd >= 1024)
        return false;
    int mask = 1 << ((unsigned long)fd % (sizeof(__int32_t) * 8));
    if (fds->fds_bits[(unsigned long)fd / (sizeof(__int32_t) * 8)] & mask)
        return true;
    return false;
}

void fdClr(int fd, fd_set *fds)
{
    if (fd < 0 || fd >= 1024)
        return ;

    int mask = ~(1 << ((unsigned long)fd % (sizeof(__int32_t) * 8)));
    fds->fds_bits[(unsigned long)fd/(sizeof(__int32_t) * 8)] &= mask;
}
  
unsigned long hToNL(unsigned long hostlong)
{
    return (
            (hostlong & 0x000000ffU) << 24 |
            (hostlong & 0x0000ff00U) << 8 |
            (hostlong & 0x00ff0000U) >> 8 |
            (hostlong & 0xff000000U) >> 24
           );
}

unsigned short hToNS(unsigned short hostshort)
{
    return ( 
            (hostshort & 0x00ffU) << 8 |
            (hostshort & 0xff00U) >> 8 
           );
}

unsigned long nToHL(unsigned long hostlong)
{
    return (
            (hostlong & 0x000000ffU) << 24 |
            (hostlong & 0x0000ff00U) << 8 |
            (hostlong & 0x00ff0000U) >> 8 |
            (hostlong & 0xff000000U) >> 24
           );
}

unsigned short nToHS(unsigned short hostshort)
{
    return (
            (hostshort & 0x00ffU) << 8 |
            (hostshort & 0xff00U) >> 8
           );
}

}