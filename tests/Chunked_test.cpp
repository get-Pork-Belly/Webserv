#include "Request.hpp"
#include "utils.hpp"
#include <iostream>

int main()
{
    std::string req = "OPTION / HTTP/1.1\r\nTransfer-Encoding: chunked\r\n\r\n5\r\nabcdef\r\na\r\n01234567890\r\n0";

	Request test;
	test.parseRequest(req);

	std::cout << "============================= result =============================" << std::endl;
    std::cout << test.getRequestMethod() << std::endl;
    std::cout << test.getRequestUri() << std::endl;
    std::cout << test.getRequestVersion() << std::endl;
    for (auto& kv : test.getRequestHeaders())
    {
        std::cout << kv.first << std::endl;
        std::cout << kv.second << std::endl;
    }
    std::cout << test.getRequestBodies() << std::endl;

}