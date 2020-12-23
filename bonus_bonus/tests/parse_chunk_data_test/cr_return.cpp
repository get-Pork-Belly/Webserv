#include <iostream>
#include <unistd.h>
#include <fcntl.h>

int main()
{
	int fd;

	fd = open("65536", O_RDWR | O_CREAT, 0666);

	for (int i = 0; i < 65536; i++)
	{
		write(fd, "a", 1);
	}
	//write(fd, "\r", 1);

	fd = open("65535_cr", O_RDWR | O_CREAT, 0666);

	for (int i = 0; i < 65535; i++)
	{
		write(fd, "a", 1);
	}
	write(fd, "\r", 1);

	fd = open("65534_crlf", O_RDWR | O_CREAT, 0666);

	for (int i = 0; i < 65534; i++)
	{
		write(fd, "a", 1);
	}
	write(fd, "\r\n", 2);

	fd = open("65535_cr_65535_cr_65534_crlf", O_RDWR | O_CREAT, 0666);

	for (int i = 0; i < 65534; i++)
	{
		write(fd, "a", 1);
	}
		write(fd, "A", 1);
	write(fd, "\r", 1);

	for (int i = 0; i < 65534; i++)
	{
		write(fd, "b", 1);
	}
		write(fd, "B", 1);
	write(fd, "\r", 1);

	for (int i = 0; i < 65533; i++)
	{
		write(fd, "c", 1);
	}
		write(fd, "C", 1);
	write(fd, "\r\n", 2);

	fd = open("65535_crlf", O_RDWR | O_CREAT, 0666);

	for (int i = 0; i < 65534; i++)
	{
		write(fd, "a", 1);
	}
		write(fd, "A", 1);
	write(fd, "\r", 1);
	write(fd, "\n", 1);

	return (0);
}
