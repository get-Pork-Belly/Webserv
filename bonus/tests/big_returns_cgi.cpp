#include <iostream>
#include <string>
#include <unistd.h>
#include <signal.h>

const int ALL = 100000000;
const int BUFFER_SIZE = 65536;

int main(int argc, char* argv[], char* envp[])
{
	signal(SIGPIPE, SIG_IGN);
    (void)argc;
    (void)argv;
    (void)envp;
    char buf[BUFFER_SIZE + 1];

    read(0, buf, BUFFER_SIZE);

    std::string status = "Status: 200 OK\r\n";
    std::string content= "Content-Type: text/html; charset=utf-8\r\n\r\n";
    write(1, status.c_str(), status.length());
    write(1, content.c_str(), content.length());

    int bytes = 0;
    for (unsigned int i = 0; i < ALL / 20; i++)
    // for (unsigned int i = 0; i < (ALL / BUFFER_SIZE) + 1; i++)
    {
        bytes += write(1, "abcdefghij0123456789", 20);
        usleep(100);
		// std::cerr<<bytes<<std::endl;
        // std::memset(reinterpret_cast<void *>(buf), 75, BUFFER_SIZE + 1);
        // bytes += write(1, buf, BUFFER_SIZE);
    }
    // std::cerr<<"\033[1;37;41m"<<"bytes: "<<bytes<<"\033[0m"<<std::endl;
    write(1, "\nend", 4);
    write(1, "\r\n", 2);
    close(1);
    return (0);
}