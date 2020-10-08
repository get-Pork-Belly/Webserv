#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include <iostream>

//TODO: 임시로 예제구현위해 선언
#define BUFFER_SIZE 100

int main(int argc, const char *argv[])
{
    int server_socket;
    struct sockaddr_in server_address;

    fd_set read_fds;
    fd_set write_fds;
    fd_set exception_fds;
    fd_set tmp;

    int fd_max; // fd순회할 때 0부터 fd_max까지 순회한다.

    //TODO: buffer
    char buf[BUFFER_SIZE];
    int len; // read의 바이트 수를 확인하기 위해. 
    struct timeval timeout;

    //TODO: 인자 예외처리
    server_socket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP); // TCP소켓 생성
    server_address.sin_family = AF_INET;

    //TODO: 엔디안 변환 함수인 htonl, htons을 직접구현.
    server_address.sin_addr.s_addr = htonl(INADDR_ANY);
    //TODO: 엔디안 변환 함수인 htonl, htons을 직접구현.
    //TODO: atoi가 사용가능한지 확인
    server_address.sin_port = htons(atoi(argv[1]));

    if (bind(server_socket, reinterpret_cast<struct sockaddr *>(&server_address), static_cast<socklen_t>(sizeof(server_address))))
    {
        //error처리
        std::cerr<<"Bind error"<<std::endl;
    }

    //TODO: listen의 두번째 인자로 적절한 값을 확인
    if (listen(server_socket, 128) == -1)
    {
        //error처리
        std::cerr<<"Listen error"<<std::endl;
    }

    //TODO: 매크로함수는 직접 구현해야함.
    FD_ZERO(&read_fds);
    FD_ZERO(&write_fds);
    FD_ZERO(&exception_fds);

    //TODO: read fd외에 write, exception에도 서버소켓 추가한 것이 어떻게 작용하는지 확인
    FD_SET(server_socket, &read_fds);
    FD_SET(server_socket, &write_fds);
    FD_SET(server_socket, &exception_fds);
    fd_max = server_socket;

    while (true)
    {
        int fd;
        int len;
        int client_socket;
        int client_len;
        struct sockaddr_in client_address;

        //TODO: 왜 tmp 변수를 써야 하는걸까        
        tmp = read_fds;

        //TODO: 적정 타임아웃값을 RFC 등을 통해 확인
        timeout.tv_sec = 5;
        timeout.tv_usec = 5;

        if (select(fd_max + 1, &tmp, 0, 0, &timeout) == -1)
            std::cerr<<"select error"<<std::endl;

        for (fd = 0; fd < fd_max + 1; fd++)
        {
            if (FD_ISSET(fd, &tmp))
            {
                if (fd == server_socket)
                {
                    client_len = sizeof(client_address);
                    if ((client_socket = accept(server_socket, reinterpret_cast<struct sockaddr *>(&client_address), reinterpret_cast<socklen_t *>(&client_len))) == -1)
                    {
                        std::cerr<<"accept error"<<std::endl;
                        //TODO: accept 실패 시 취해야 하는 행동
                    }
                    FD_SET(client_socket, &read_fds); // readfds에서 클라이언트 소켓의 값을 1로 바꿔줌
                    if (fd_max < client_socket)
                        fd_max = client_socket;
                    std::cout<<"Client Connect FD = "<<client_socket<<std::endl;
                }
                else
                {
                    if ((len = read(fd, buf, BUFFER_SIZE)) < 0)
                    {
                        std::cerr<<"read error"<<std::endl;
                    }
                    if (len == 0)
                    {
                        FD_CLR(fd, &read_fds); // read_fds에서 fd를 0으로 만든다.
                        close(fd);
                        std::cout<<"Client Disconnect: "<<fd<<std::endl;
                    }
                    else
                    {
                        write(fd, buf, len);
                    }                    
                }
            }
        }
    }

    return (0);
}