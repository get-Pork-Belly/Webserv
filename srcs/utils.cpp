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

std::string ltrim(const std::string& str, const char *token="\t\n ")
{
	size_t n = str.find_first_not_of(token);
	return (n == std::string::npos ? str : str.substr(n, str.length()));
}

std::string rtrim(const std::string& str, const char *token="\t\n ")
{
	size_t n = str.find_last_not_of(token);
	return (n == std::string::npos ? str : str.substr(0, n + 1));
}

}
