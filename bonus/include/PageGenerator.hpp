#ifndef PAGEGENERATOR_HPP
# define PAGEGENERATOR_HPP

# include "Response.hpp"
# include <iostream>
# include <string>

class PageGenerator
{
private:
    /* data */
    PageGenerator();
    PageGenerator(const PageGenerator& other);
    PageGenerator& operator=(const PageGenerator& rhs);
public:
    /* Getter */
    /* Setter */
    /* Exception */
    /* Util */
    static void makeErrorPage(Response& res);
    static void makeAutoIndex(Response& res);
};

#endif
