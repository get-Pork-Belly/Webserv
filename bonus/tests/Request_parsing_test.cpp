#include "Request.hpp"

#include <iostream>
#include <string>
#include <map>
#include <vector>
#include "utils.hpp"

int main()
{
    std::string req = "OPTION / HTTP/1.1\r\nHost: 127.0.0.1:8080\r\nUser-Agent: Mozilla/5.0 (Macintosh;) Firefox/51.0\r\nAccept: text/html\r\nAccept-Language: en-US\r\nAccept-Encoding: gzip\r\nConnection: keep-alive\r\nContent-Type: multipart/form-data\r\nContent-Length: 345\r\n\r\nHelloWorld, everybody, buddy, whatsup\r\n";

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
    std::cout << test.getRequestBody() << std::endl;

}