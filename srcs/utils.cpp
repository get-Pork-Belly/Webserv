/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: iwoo <iwoo@student.42seoul.kr>             +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2020/10/13 09:51:39 by sanam             #+#    #+#             */
/*   Updated: 2020/10/13 21:43:43 by iwoo             ###   ########.fr       */
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
				if (tmp.length())
					result.push_back(tmp);
				str = str.substr(index + token.size());
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