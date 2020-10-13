#include <string>
#include <vector>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <iostream>
#include "utils.hpp"

const int BUF_SIZE = 4096;

void convertFileToStringVector(const char *config_file_path)
{
	int							fd;
	int							readed;
	char						buf[BUF_SIZE];
	std::string 				readed_string;
	std::vector<std::string>	lines;

	fd = open(config_file_path, O_RDONLY, 0644);
	if (fd < 0)
		throw (strerror(errno));
	memset(reinterpret_cast<void *>(buf), 0, BUF_SIZE);

	while ((readed = read(fd, reinterpret_cast<void *>(buf), BUF_SIZE)))
	{
		if (readed < 0)
			throw(strerror(errno));
		readed_string += std::string(buf);
	}

	lines = ft::split(readed_string, "\n");

	// for (std::string line : lines)
	// 	std::cout << line << std::endl;

	for (std::string line : lines)
	{
		// TODO trimmed 데이터가 유지될까? 포인터로 만들어야하지 않을까?
		// TODO push_back의 인자로 std::string(trimmed) 처럼 새롭게 만들어서 넣을까?
		std::string trimmed = ft::ltrim(ft::rtrim(line));
		std::cout<<trimmed<<std::endl;
		// if (trimmed.size() > 0)
		// 	_str_vector_configfile.push_back(std::string(trimmed));
	}
}

int main(void)
{
	try
	{
		convertFileToStringVector("./tests/test.test");
	}
	catch(const char *e)
	{
		std::cout<<e<<std::endl;
	}
	

	return (0);
}