#include <iostream>
#include <string>
#include "utils.hpp"

int main()
{
    std::string empty = "";
    std::string normal = "abcd";
    std::string longg = "asfajsdhflakshfljashfashdfaklsjhflaskhfajlsdhfklajshflkjsadhf";

    char *e = ft::strdup(empty);
    std::cout << "empty: [" << e << "]" << std::endl;
    char *n = ft::strdup(normal);
    std::cout << "normal: [" << n << "]" << std::endl;
    char *l = ft::strdup(longg);
    std::cout << "long: ["  << l << "]"<< std::endl;
}
