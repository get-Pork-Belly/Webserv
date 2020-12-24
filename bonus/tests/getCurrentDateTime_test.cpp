#include "utils.hpp"
#include <iostream>
#include <ctime>

namespace test 
{

std::string STDgetCurrentDateTime()
{
    time_t raw_time;
    struct tm* ptm;

    time(&raw_time);
    ptm = gmtime(&raw_time);
    char buf[64];
    const char* fmt = "%a, %d %b %Y %X %Z";

    ft::memset(buf, 0, sizeof(buf));
    strftime(buf, sizeof(buf), fmt, ptm);
    std::string gmt_time(buf);

    return (buf);
};

}

int main()
{
    std::cout<<"GMT == UTC != KST"<<std::endl;
    std::cout<<"ft:  "<<ft::getCurrentDateTime()<<std::endl;
    std::cout<<"std: "<<test::STDgetCurrentDateTime()<<std::endl;

    return (0);
}