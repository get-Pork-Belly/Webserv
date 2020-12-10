#include "utils.hpp"
#include <iomanip>
#include <iostream>

namespace ft {

std::vector<std::string>
split(std::string str, std::string const &token)
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

std::string
ltrim(const std::string& str, const char *token)
{
    size_t n = str.find_first_not_of(token);
    return (n == std::string::npos ? str : str.substr(n, str.length()));
}

std::string
rtrim(const std::string& str, const char *token)
{
    size_t n = str.find_last_not_of(token);
    return (n == std::string::npos ? str : str.substr(0, n + 1));
}

void
fdZero(fd_set *fds)
{
    for (int i = 0; i < 32; ++i)
        fds->fds_bits[i] = 0;
}

void
fdSet(int fd, fd_set *fds)
{
    if (fd < 0 || fd >= 1024)
        return ;
    int mask = 1 <<((unsigned long)fd % (sizeof(__int32_t) * 8));
    fds->fds_bits[(unsigned long)fd/(sizeof(__int32_t) * 8)] |= mask;
}

bool
fdIsCopySet(int fd, fd_set *fds)
{
    if (fd < 0 || fd >= 1024)
        return false;
    int mask = 1 << ((unsigned long)fd % (sizeof(__int32_t) * 8));
    if (fds->fds_bits[(unsigned long)fd / (sizeof(__int32_t) * 8)] & mask)
        return true;
    return false;
}

void
fdClr(int fd, fd_set *fds)
{
    if (fd < 0 || fd >= 1024)
        return ;

    int mask = ~(1 << ((unsigned long)fd % (sizeof(__int32_t) * 8)));
    fds->fds_bits[(unsigned long)fd/(sizeof(__int32_t) * 8)] &= mask;
}
  
unsigned long
hToNL(unsigned long hostlong)
{
    return (
            (hostlong & 0x000000ffU) << 24 |
            (hostlong & 0x0000ff00U) << 8 |
            (hostlong & 0x00ff0000U) >> 8 |
            (hostlong & 0xff000000U) >> 24
           );
}

unsigned short
hToNS(unsigned short hostshort)
{
    return ( 
            (hostshort & 0x00ffU) << 8 |
            (hostshort & 0xff00U) >> 8 
           );
}

unsigned long
nToHL(unsigned long hostlong)
{
    return (
            (hostlong & 0x000000ffU) << 24 |
            (hostlong & 0x0000ff00U) << 8 |
            (hostlong & 0x00ff0000U) >> 8 |
            (hostlong & 0xff000000U) >> 24
           );
}

unsigned short
nToHS(unsigned short hostshort)
{
    return (
            (hostshort & 0x00ffU) << 8 |
            (hostshort & 0xff00U) >> 8
           );
}

bool
substr(std::string &line, std::string &lines, const std::string &delim)
{
    size_t index;

    if ((index = lines.find(delim)) != std::string::npos)
    {
        line = lines.substr(0, index);
        lines = lines.substr(index + delim.size());
    }
    else
    {
        line = lines;
        lines = "";
        return (false);
    }
    return (true);
}

void*
memset(void* b, int c, size_t len)
{
    size_t i = 0;
    unsigned char* copy = (unsigned char*)b;

    while (i++ < len)
        *copy++ = c;
    return (b);
}


//TODO: stoiHexToDec
int
stoiHex(const std::string& str)
{
    int ret = 0;
    int i = 0;
    size_t pos = 0;
    const std::string digit = "0123456789abcdef";

    if (str.empty() || digit.find(str[i]) == std::string::npos)
        return (-1);
    while ((pos = digit.find(str[i])) != std::string::npos)
    {
        ret *= 16;
        ret += pos;
        i++;
    }
    return (ret);
}

//TODO: stoiDecToHex

int
getTimeDiffBetweenGMT(char *time_zone)
{
    std::map<std::string, int> time_diff_between_gmt = {
        {"KST", 9 * 60 * 60},
        {"JST", 9 * 60 * 60},
        {"CTT", 8 * 60 * 60},
        {"ECT", 1 * 60 * 60},
        {"PST", -7 * 60 * 60},
    };
    return (time_diff_between_gmt[std::string(time_zone)]);
}

std::string 
getCurrentDateTime()
{
    struct tm time;
    struct timeval tv;
    char buf[64];
    const char* fmt = "%a, %d %b %Y %X GMT";

    ft::memset(buf, 0, sizeof(buf));
    gettimeofday(&tv, NULL);
    strptime(std::to_string(tv.tv_sec).c_str(), "%s", &time);
    tv.tv_sec -= getTimeDiffBetweenGMT(time.tm_zone);
    strptime(std::to_string(tv.tv_sec).c_str(), "%s", &time);
    strftime(buf, sizeof(buf), fmt, &time);
    return (buf);
}

std::string
getEstimatedUnavailableTime()
{
    struct tm time;
    struct timeval tv;
    char buf[64];
    const char* fmt = "%a, %d %b %Y %X GMT";
    int estimated_unavailable_time = 86400;

    ft::memset(buf, 0, sizeof(buf));
    gettimeofday(&tv, NULL);
    tv.tv_sec += estimated_unavailable_time;
    strptime(std::to_string(estimated_unavailable_time).c_str(), "%s", &time);
    tv.tv_sec -= getTimeDiffBetweenGMT(time.tm_zone);
    strptime(std::to_string(tv.tv_sec).c_str(), "%s", &time);
    strftime(buf, sizeof(buf), fmt, &time);
    return (buf);
}

size_t
strlen(const char* str)
{
    size_t i = 0;
    while (str[i])
        i++;
    return (i);
}

char*
strdup(const std::string& s)
{
	size_t		i = -1;
	size_t		len = s.length();
	char*		ret;

	len = s.length();
	if (!(ret = (char *)malloc(sizeof(char) * (len + 1))))
        return (NULL);
	while (++i < len)
		ret[i] = s[i];
	ret[len] = '\0';
	return (ret);
}

std::string
inetNtoA(const in_addr_t& client_address)
{
     return (std::to_string(client_address % 256) + "."
            + std::to_string((client_address / 256) % 256) + "."
            + std::to_string((client_address / 256 / 256) % 256) + "."
            + std::to_string((client_address / 256 / 256 / 256)));
}

void
doubleFree(char*** target)
{
    for (int i = 0; (*target)[i]; i++)
        free((*target)[i]);
    free(*target);
    *target = nullptr;
}

static unsigned int
nbrlenHex(unsigned int n)
{
	int	len;

	len = 1;
	while (n >= 16)
	{
		n /= 16;
		len++;
	}
	return (len);
}

std::string
itosHex(unsigned int n)
{
    const char* base = "0123456789ABCDEF";
    size_t len = ft::nbrlenHex(n);
    std::string result;

    for (size_t i = 0; i < len; i++)
    {
        result += base[n % 16];
        n /= 16;
    }
    std::reverse(result.begin(), result.end());
    return (result);
}

bool
IsPrintable(char ch)
{
	if (ch  >= 32 && ch <= 126)
		return (true);
	return (false);
}

bool
fileExists(std::string& file_path)
{
    DIR* dir_ptr;
    if ((dir_ptr = opendir(file_path.c_str())) != NULL)
    {
        //NOTE: file exists but directory
        closedir(dir_ptr);
        return (true);
    }
    else
    {
        if (errno == ENOENT)
            return (false);
    }
    return (true);
}

std::vector<std::string>
makeDirectoryEntry(DIR* dir_ptr)
{
    std::vector<std::string> directory_entry;
    struct dirent* entry = NULL;
    while ((entry = readdir(dir_ptr)) != NULL)
    {
        std::string file_path(entry->d_name);
        if (entry->d_type == 4)
            file_path += "/";
        directory_entry.push_back(file_path);
    }
    return (directory_entry);
}


}
