#include "utils.hpp"
#include <iostream>

int main()
{
	std::string test1 = "abc";
	std::string test2 = "z123";
	std::string test3 = "1a87";
	std::string test4 = "0";
	std::string test5 = "az";

	std::cout << "Test1: " << ft::stoiHex(test1) << std::endl;
	std::cout << "Test2: " << ft::stoiHex(test2) << std::endl;
	std::cout << "Test3: " << ft::stoiHex(test3) << std::endl;
	std::cout << "Test4: " << ft::stoiHex(test4) << std::endl;
	std::cout << "Test5: " << ft::stoiHex(test5) << std::endl;
}