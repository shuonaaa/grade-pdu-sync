#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>

#define PORT_SERVER 8888
#define LENGTH 100

void server_process(int fd, struct sockaddr* addr) {
    int n;
    int len;
    char tmp_buff[LENGTH];
    while (1)
    {
        len = sizeof(*addr);
        n = recvfrom(fd , tmp_buff , LENGTH, 0 , addr, &len);
        printf("%s\n", tmp_buff);
    }
}

int main(int agrc , char* argv[]) {
    int fd_serv;
    struct sockaddr_in addr_serv , addr_clie;
    fd_serv = socket(AF_INET,SOCK_DGRAM,0);

    memset(&addr_serv, 0 , sizeof(addr_serv));
    addr_serv.sin_family = AF_INET;
    inet_aton("127.0.0.1", &addr_serv.sin_addr);
    printf("IP address is: %s\n",inet_ntoa(addr_serv.sin_addr));
    addr_serv.sin_port = htons(PORT_SERVER);

    bind(fd_serv , (struct sockaddr*)&addr_serv , sizeof(addr_serv));
    server_process(fd_serv , (struct sockaddr*)&addr_clie);

    exit(0);
}