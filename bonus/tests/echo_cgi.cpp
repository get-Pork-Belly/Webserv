#include <iostream>
#include <string>
#include <unistd.h>

const int ALL = 100000000;
const int BUFFER_SIZE = 65536;

int main(int argc, char* argv[], char* envp[])
{
    (void)argc;
    (void)argv;
    (void)envp;
    char buf[BUFFER_SIZE + 1];


    std::string status = "Status: 200 OK\r\n";
    std::string content = "Content-Type: image/jpeg; charset=utf-8\r\n\r\n";
    write(1, status.c_str(), status.length());
    write(1, content.c_str(), content.length());

	int bytes;
    std::memset(reinterpret_cast<void *>(buf), 0, BUFFER_SIZE);
    while ((bytes = read(0, buf, BUFFER_SIZE)) > 0)
    {
        write(1, buf, bytes);
    }
    write(1, "\r\n", 2);

    close(1);
    return (0);
}