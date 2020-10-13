/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: iwoo <iwoo@student.42seoul.kr>             +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2020/10/13 09:51:39 by sanam             #+#    #+#             */
/*   Updated: 2020/10/14 00:05:55 by iwoo             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "utils.hpp"

namespace ft
{
std::vector<std::string> split(std::string str, std::string const &token)
{
	std::vector<std::string>	result;
	int							index;
	std::string 				tmp;

	while (str.size())
	{
		index = str.find(token);
		if (static_cast<size_t>(index) != std::string::npos)
		{
			tmp = str.substr(0, index);
			if (tmp.size())
				result.push_back(tmp);
			str = str.substr(index + token.size());
			// if (str.size() == 0)
			// 	result.push_back(str);
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

}