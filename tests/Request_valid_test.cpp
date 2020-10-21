#include "Request.hpp"

#include <iostream>
#include <string>
#include <map>
#include <vector>
#include "utils.hpp"

int main()
{
    //TODO: 중복검사 다시 진행해야함.
    // 중복 키 //
    // std::string req = "OPTION / HTTP/1.1\r\nHost: 127.0.0.1:8080\r\nHost: 127\r\nUser-Agent: Mozilla/5.0 (Macintosh;) Firefox/51.0\r\nAccept-Charsets: text/html\r\nAccept-Language: en-US\r\nContent-Type: multipart/form-data\r\nContent-Length: 345\r\n\r\nHelloWorld, everybody, buddy, whatsup\r\n";

    // 없는 헤더 //NOTE: 완료
    // std::string req = "OPTION / HTTP/1.1\r\nHost: 127.0.0.1:8080\r\nContents: 127\r\nUser-Agent: Mozilla/5.0 (Macintosh;) Firefox/51.0\r\nAccept-Charsets: text/html\r\nAccept-Language: en-US\r\nContent-Type: multipart/form-data\r\nContent-Length: 345\r\n\r\nHelloWorld, everybody, buddy, whatsup\r\n";

    //TODO: 해야되는지 잘 모르겠음... 논의필요!
    // 헤더필드 CRLF 두 번
    // std::string req = "OPTION / HTTP/1.1\r\nHost: 127.0.0.1:8080\r\n\r\nUser-Agent: Mozilla/5.0 (Macintosh;) Firefox/51.0\r\nAccept-Charsets: text/html\r\nAccept-Language: en-US\r\nContent-Type: multipart/form-data\r\nContent-Length: 345\r\n\r\nHelloWorld, everybody, buddy, whatsup\r\n";
    //TODO: 헤더 맵 밸류 안에 헤더 키들이 있는지 조사하는 걸 해보자.. 그러기 위해서는 음,, 헤더키들을 enum으로 저장시켜버릴까..?
    // CRLF가 아닌 다른 문자로 구분되어질 때 (CR 만있다던가..)
    // std::string req = "OPTION / HTTP/1.1\r\nHost: 127.0.0.1:8080User-Agent: Mozilla/5.0 (Macintosh;) Firefox/51.0\r\nAccept-Charsets: text/html\r\nAccept-Language: en-US\r\nContent-Type: multipart/form-data\r\nContent-Length: 345\r\n\r\nHelloWorld, everybody, buddy, whatsup\r\n";

    // 헤더 이름 앞, 헤더이름과 콜론 사이 공백 //NOTE: 완료
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