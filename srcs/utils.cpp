/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sanam <marvin@42.fr>                       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2020/10/13 09:51:39 by sanam             #+#    #+#             */
/*   Updated: 2020/10/13 10:15:40 by sanam            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "utils.hpp"

namespace ft
{
	std::vector<std::string> split(std::string str, std::string const &token)
	{
		std::vector<std::string>	result;
		int							index;

		while (str.size())
		{
			index = str.find(token);
			if (index != std::string::npos)
			{
				result.push_back(str.substr(0, index));
				str = str.substr(index + token.size());
				if (str.size() == 0)
					result.push_back(str);
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
