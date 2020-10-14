#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <iostream>

int main (int argc, char * argv[])
{

 unsigned short host_port = 0x1234;      //2byte 데이터 저장
 unsigned short net_port;
 unsigned long host_addr = 0x12345678; // 4byte 데이터 저장
 unsigned long net_addr;

 net_port = htons(host_port);                //리틀엔디안에서 빅 엔디안 방식으로 net_port 에 저장
 net_addr = htonl(host_addr);               //리틀엔디안에서 빅 엔디안 방식으로 net_addr 에 저장

 printf("Host ordered port : %#x \n" , host_port);
 printf("Network ordered port : %#x \n" , net_port);
 printf("Host ordered address : %#lx \n ", host_addr);
 printf("Network ordered address : %#lx \n" , net_addr);

//  net_port = ft::htons(host_port);
//  net_addr = ft::htonl(host_addr);

 std::cout<<"Host ordered port : "<<host_port<<std::endl;
 std::cout<<"Network ordered port : "<<net_port<<std::endl;
 std::cout<<"Host ordered address : "<<host_addr<<std::endl;
 std::cout<<"Network ordered address : "<<net_addr<<std::endl;


 return 0;

}