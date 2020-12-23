#include "utils.hpp"
#include <iostream>

//NOTE: hToNL 함수를 거치고 나면 inetNtoA 함수에서 ipv4 주소가 거꾸로 출력됩니다. 사용 시 유의하시기 바랍니다.

int main()
{
    unsigned long ip_addr_test1 = ft::hToNL(0x1020304); // 1.2.3.4
    unsigned long ip_addr_test2 = 0xb90eac0; // 192.234.144.11
    unsigned long ip_addr_test3 = 0x6601a8c0; // 192.168.1.192
    unsigned long ip_addr_test4 = 0x100007f; // 127.0.0.1

    std::cout << "IP Address: " << ft::inetNtoA(ip_addr_test1) << std::endl;
    std::cout << "IP Address: " << ft::inetNtoA(ip_addr_test2) << std::endl;
    std::cout << "IP Address: " << ft::inetNtoA(ip_addr_test3) << std::endl;
    std::cout << "IP Address: " << ft::inetNtoA(ip_addr_test4) << std::endl;

    return (0);
}