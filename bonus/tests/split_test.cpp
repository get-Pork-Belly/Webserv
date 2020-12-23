#include <iostream>
#include "utils.hpp"

int main(void)
{

std::vector<std::string> res;
{
    std::string test1 = "   abc def ghi  i   ";

    res = ft::split(test1, std::string(" "));

    for (auto& i : res)
    {
        std::cout<<i<<std::endl;
    }
}

{
    std::cout<<"==============================="<<std::endl;
    std::string test2 = " abc adb abg abf abh ";
    res = ft::split(test2, "ab");

    for (auto& i : res)
    {
        std::cout<<i<<std::endl;
    }
}

{
    std::cout<<"==============================="<<std::endl;
    std::string test = " server {\n abc\n adb\n abg\n abf\n abh\n}\n";
    res = ft::split(test, "\n");

    for (auto& i : res)
    {
        std::cout<<i<<std::endl;
    }
}

{
    std::cout<<"==============================="<<std::endl;
    std::cout<<"blank case"<<std::endl;
    std::string test = "";
    res = ft::split(test, "\n");

    for (auto& i : res)
    {
        std::cout<<i<<std::endl;
    }
}



    return (0);
}