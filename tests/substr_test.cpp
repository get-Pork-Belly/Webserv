#include "utils.hpp"
#include <iostream>

int main()
{
    std::string lines = "HTTP/1.1 200 OK\r\nServer: nginx/1.2.1\r\nContent-Type: text/html\r\nContent-Length: 8\r\nConnection: keep-alive\r\n\r\n<html />";
    std::string line;

    std::cout << "=========== Origin lines ===========" << std::endl;
    std::cout << lines << std::endl << std::endl;

    std::cout << "=========== 1 line ===========" << std::endl;
    ft::substr(line, lines, "\r\n");
    std::cout << line << std::endl;
    std::cout << "=========== 1 substr after lines ===========" << std::endl;
    std::cout << lines << std::endl << std::endl;
    
    std::cout << "=========== 2 line ===========" << std::endl;
    ft::substr(line, lines, "\r\n");
    std::cout << line << std::endl;
    std::cout << "=========== 2 substr after lines ===========" << std::endl;
    std::cout << lines << std::endl << std::endl;

    std::cout << "=========== 3 line ===========" << std::endl;
    ft::substr(line, lines, "\r\n");
    std::cout << line << std::endl;
    std::cout << "=========== 3 substr after lines ===========" << std::endl;
    std::cout << lines << std::endl << std::endl;

    std::cout << "=========== 4 line ===========" << std::endl;
    ft::substr(line, lines, "\r\n");
    std::cout << line << std::endl;
    std::cout << "=========== 4 substr after lines ===========" << std::endl;
    std::cout << lines << std::endl << std::endl;

    std::cout << "=========== 5 line ===========" << std::endl;
    ft::substr(line, lines, "\r\n");
    std::cout << line << std::endl;
    std::cout << "=========== 5 substr after lines ===========" << std::endl;
    std::cout << lines << std::endl << std::endl;

    std::cout << "=========== 6 line ===========" << std::endl;
    ft::substr(line, lines, "\r\n");
    std::cout << line << std::endl;
    std::cout << "=========== 6 substr after lines ===========" << std::endl;
    std::cout << lines << std::endl << std::endl;

    std::cout << "=========== 7 line ===========" << std::endl;
    ft::substr(line, lines, "\r\n");
    std::cout << line << std::endl;
    std::cout << "=========== 7 substr after lines ===========" << std::endl;
    std::cout << lines << std::endl << std::endl;

    std::cout << "=========== 8 line ===========" << std::endl;
    ft::substr(line, lines, "\r\n");
    std::cout << line << std::endl;
    std::cout << "=========== 8 substr after lines ===========" << std::endl;
    std::cout << lines << std::endl << std::endl;
}