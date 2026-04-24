#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>

#define PORT_SERV 8888
#define LENGTH 100

void client_process(int fd, struct sockaddr* addr) {
    int n;
    int len;
    char tmp_buff[LENGTH];
    strncpy(tmp_buff, "hello!" , sizeof("hello!"));
    len = sizeof(*addr);
    n = sendto(fd , tmp_buff , LENGTH, 0 , addr, len);  
    // printf("%s", tmp_buff);
}

int main(int agrc , char* argv[]) {
    int fd;
    struct sockaddr_in addr_serv;

    fd = socket(AF_INET,SOCK_DGRAM,0);
    memset(&addr_serv,0,sizeof(addr_serv));
    addr_serv.sin_family = AF_INET;
    inet_aton("127.0.0.1", &addr_serv.sin_addr);
    printf("IP address is: %s\n",inet_ntoa(addr_serv.sin_addr));
    addr_serv.sin_port = htons(PORT_SERV);

    client_process(fd, (struct sockaddr*)&addr_serv);

    exit(0);
}