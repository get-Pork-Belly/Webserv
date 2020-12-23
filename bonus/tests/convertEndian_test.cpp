#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <iostream>
#include "utils.hpp"

int main (int argc, char * argv[])
{

(void)argc;
(void)argv;
 unsigned short host_port = 0x1234;      //2byte 데이터 저장
 unsigned short reconvert_host_port;
 unsigned short net_port;
 unsigned long host_addr = 0x12345678; // 4byte 데이터 저장
 unsigned long reconvert_host_addr;
 unsigned long net_addr;

 net_port = htons(host_port);                //리틀엔디안에서 빅 엔디안 방식으로 net_port 에 저장
 net_addr = htonl(host_addr);               //리틀엔디안에서 빅 엔디안 방식으로 net_addr 에 저장
 reconvert_host_port = ntohs(net_port);     //빅엔디안에서 리틀 엔디안 방식으로 다시 변경
 reconvert_host_addr = ntohl(net_addr);     //빅엔디안에서 리틀 엔디안 방식으로 다시 변경

std::cout<<"--------std---------"<<std::endl;
 printf("Host ordered port : %#x \n" , host_port);
 printf("Network ordered port : %#x \n" , net_port);
 printf("Host ordered port(reconverted) : %#x \n" , reconvert_host_port);
 printf("Host ordered address : %#lx \n ", host_addr);
 printf("Network ordered address : %#lx \n" , net_addr);
 printf("Host ordered addr(reconverted) : %#lx \n" , reconvert_host_addr);

std::cout<<"--------ft---------"<<std::endl;
 net_port = ft::hToNS(host_port);
 net_addr = ft::hToNL(host_addr);
 reconvert_host_port = ft::nToHS(net_port);     //빅엔디안에서 리틀 엔디안 방식으로 다시 변경
 reconvert_host_addr = ft::nToHL(net_addr);     //빅엔디안에서 리틀 엔디안 방식으로 다시 변경

 printf("Host ordered port : %#x \n" , host_port);
 printf("Network ordered port : %#x \n" , net_port);
 printf("Host ordered port(reconverted) : %#x \n" , reconvert_host_port);
 printf("Host ordered address : %#lx \n ", host_addr);
 printf("Network ordered address : %#lx \n" , net_addr);
 printf("Host ordered addr(reconverted) : %#lx \n" , reconvert_host_addr);



 return 0;

}

// #include <sys/time.h>
// #include <sys/socket.h>
// #include <fcntl.h>
// #include <unistd.h>
// #include <sys/types.h>
// #include <sys/stat.h>
// #include <unistd.h>
// #include <stdlib.h>
// #include <stdio.h>
// #include <netinet/in.h>
// #include <arpa/inet.h>

// int main(int argc, char **argv)
// {
//     int client_sockfd;
//     struct sockaddr_in clientaddr;
//     int data = 123456789;
//     int client_len;

//     client_sockfd = socket(AF_INET, SOCK_STREAM, 0);
//     clientaddr.sin_family = AF_INET;
//     clientaddr.sin_addr.s_addr = inet_addr("192.168.100.190");
//     clientaddr.sin_port = htons(atoi(argv[1]));

//     client_len = sizeof(clientaddr);

//     if (connect(client_sockfd, (struct sockaddr *)&clientaddr, client_len) < 0)
//     {
//         perror("Connect error : ");
//         exit(0);
//     }
    
//     // 보낼 데이터 네트워크 byte order를 따르도록 변경한다. 
//     data = htonl(data);
//     write(client_sockfd, (void *)&data, sizeof(int));

//     // 읽어들인 데이터는 호스트 byte order을 따르도록 변경한다.
//     read(client_sockfd, (void *)&data, sizeof(int));
//     data = ntohl(data);
//     close(client_sockfd);
// }