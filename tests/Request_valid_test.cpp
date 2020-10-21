#include "Request.hpp"

#include <iostream>
#include <string>
#include <map>
#include <vector>
#include "utils.hpp"

int main()
{
    // 중복 키 //
    // std::string req = "OPTION / HTTP/1.1\r\nHost: 127.0.0.1:8080\r\nHost: 127\r\nUser-Agent: Mozilla/5.0 (Macintosh;) Firefox/51.0\r\nAccept-Charsets: text/html\r\nAccept-Language: en-US\r\nContent-Type: multipart/form-data\r\nContent-Length: 345\r\n\r\nHelloWorld, everybody, buddy, whatsup\r\n";

    //TODO: Headers Value에 대한 Valid 체크를 진행해야 함.
    //NOTE: 헤더필드 CRLF 두 번(이 때는 처리됨) 또는 CRLF가 아닌 다른 문자로 구분되어질 때(이 때 처리 가능하도록 변경해야함.)
    // std::string req = "OPTION / HTTP/1.1\r\nHost: 127.0.0.1:8080\r\n\r\nUser-Agent: Mozilla/5.0 (Macintosh;) Firefox/51.0\r\nAccept-Charsets: text/html\r\nAccept-Language: en-US\r\nContent-Type: multipart/form-data\r\nContent-Length: 345\r\n\r\nHelloWorld, everybody, buddy, whatsup\r\n";
    // std::string req = "OPTION / HTTP/1.1\r\nHost: 127.0.0.1:8080User-Agent: Mozilla/5.0 (Macintosh;) Firefox/51.0\r\nAccept-Charsets: text/html\r\nAccept-Language: en-US\r\nContent-Type: multipart/form-data\r\nContent-Length: 345\r\n\r\nHelloWorld, everybody, buddy, whatsup\r\n";


    // 헤더 이름 앞, 헤더이름과 콜론 사이 공백
    // std::string req = "OPTION / HTTP/1.1\r\nHost : 127.0.0.1:8080\r\nUser-Agent: Mozilla/5.0 (Macintosh;) Firefox/51.0\r\nAccept-Charsets: text/html\r\nAccept-Language: en-US\r\nContent-Type: multipart/form-data\r\nContent-Length: 345\r\n\r\nHelloWorld, everybody, buddy, whatsup\r\n";

    // 문제없는 request
    std::string req = "OPTION / HTTP/1.1\r\nHost: 127.0.0.1:8080\r\nUser-Agent: Mozilla/5.0 (Macintosh;) Firefox/51.0\r\nAccept-Charsets: text/html\r\nAccept-Language: en-US\r\nContent-Type: multipart/form-data\r\nContent-Length: 345\r\n\r\nHelloWorld, everybody, buddy, whatsup\r\n";

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