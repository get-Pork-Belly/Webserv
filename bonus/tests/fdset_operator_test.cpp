#include "utils.hpp"
#include <iostream>
#include <bitset>
#include <fcntl.h>

int main()
{
	fd_set test;
	// 0 0 0 1 0 0 0 0 0 0 .... 32개
	int fd = open("./tests/test", O_RDONLY);
	int fd2 = open("./tests/test2", O_RDONLY);
	
	ft::fdZero(&test);
	std::cout << "================= FD_ZERO =================" << std::endl;
	std::cout << *(test.fds_bits) << std::endl << std::endl;

	try
	{
		ft::fdSet(fd, &test);
		ft::fdSet(fd2, &test);
		std::cout << "================= FD_SET =================" << std::endl;
		std::cout << *(test.fds_bits) << std::endl;
		//std::string temp = b.to_string();
		//std::cout << "temp: " << temp;
	}
	catch(const std::exception& e)
	{
		std::cerr << e.what() << '\n';
	}
	


	// ft::fdSet(1, &test);

	if (ft::fdIsset(fd, &test))
	{
		std::cout << "FD is Set!!!" << std::endl;
		std::cout << "================= FD_ISSET =================" << std::endl;
		std::cout << *(test.fds_bits) << std::endl << std::endl;
	}
	else
		std::cout << "FD is not Set!!!" << std::endl << std::endl;

	ft::fdClr(fd, &test);
	std::cout << "================= FD_CLR =================" << std::endl;
	std::cout << *(test.fds_bits) << std::endl << std::endl;

}