#include <iostream>
#include "utils.hpp"

int main(void)
{
    std::string test1 = "   abc def ghi  i   ";
    // expected: [abc] [def] [ghi] [i]
    std::vector<std::string> res;

    res = ft::split(test1, std::string(" "));

    for (auto& i : res)
    {
        std::cout<<i<<std::endl;
    }

    std::cout<<"==============================="<<std::endl;
    std::string test2 = " abc adb abg abf abh ";
    //expected: [ ] [c adb ] [g ] [f ] [h ]
    res = ft::split(test2, "ab");

    for (auto& i : res)
    {
        std::cout<<i<<std::endl;
    }
    return (0);
}