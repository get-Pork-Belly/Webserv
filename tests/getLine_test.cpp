#include "utils.hpp"
#include <iostream>

int main()
{
    std::string lines = "HTTP/1.1 200 OK\r\nServer: nginx/1.2.1\r\nContent-Type: text/html\r\nContent-Length: 8\r\nConnection: keep-alive\r\n\r\n<html />";
    std::string line;

    std::cout << "=========== Origin lines ===========" << std::endl;
    std::cout << lines << std::endl << std::endl;

    std::cout << "=========== first line ===========" << std::endl;
    line = ft::getLine(lines, "\r\n");
    std::cout << line << std::endl;
    std::cout << "=========== first getline after lines ===========" << std::endl;
    std::cout << lines << std::endl << std::endl;
    
    std::cout << "=========== second line ===========" << std::endl;
    line = ft::getLine(lines, "\r\n");
    std::cout << line << std::endl;
    std::cout << "=========== second getline after lines ===========" << std::endl;
    std::cout << lines << std::endl << std::endl;

    std::cout << "=========== third line ===========" << std::endl;
    line = ft::getLine(lines, "\r\n");
    std::cout << line << std::endl;
    std::cout << "=========== third getline after lines ===========" << std::endl;
    std::cout << lines << std::endl << std::endl;
}