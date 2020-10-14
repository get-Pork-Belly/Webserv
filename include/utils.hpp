#ifndef UTILS_HPP
# define UTILS_HPP

# include <vector>
# include <string>

namespace ft
{
    std::vector<std::string> split(std::string str, std::string const &token);
    std::string	ltrim(const std::string& str, char const *token = "\t\n ");
    std::string	rtrim(const std::string& str, char const *token = "\t\n ");
}

#endif
