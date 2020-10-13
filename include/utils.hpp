/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils.hpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sanam <marvin@42.fr>                       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2020/10/13 09:50:07 by sanam             #+#    #+#             */
/*   Updated: 2020/10/13 15:46:14 by sanam            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef UTILS_HPP
# define UTILS_HPP

# include <vector>
# include <string>

namespace ft
{
	std::vector<std::string>	split(std::string str, std::string const &token);
	std::string					ltrim(const std::string str, const char *token);
	std::string					rtrim(const std::string str, const char *token);
}

#endif
