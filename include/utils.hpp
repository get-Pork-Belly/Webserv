/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils.hpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: iwoo <iwoo@student.42.fr>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2020/10/13 09:50:07 by sanam             #+#    #+#             */
/*   Updated: 2020/10/14 11:10:40 by iwoo             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

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
