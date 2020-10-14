#include "utils.hpp"

namespace ft
{
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
}