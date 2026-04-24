#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include "PDULib/controlPDU.h"
#include "PDULib/dataPDU.h"

#define PORT_SERVER 8888
#define LENGTH 32

void ProcessPDU(char* buf) {
    uint8_t type = (uint8_t)buf[0];

    switch (type)
    {
    case PDU_TYPE_CONTROL: {
        ControlPDU CP = BtoControlPDU(buf);
        Control C = contPTC(&CP);
        debugPrintf("======================\n");
        debugPrintf("[ ControlPDU ]\n");
        debugPrintf("学号:   %s\n", C.sid);
        debugPrintf("课程号: %s\n", C.CourseNumber);
        debugPrintf("学期:   %s\n", C.P);
        debugPrintf("状态:   %s\n", C.status);
        debugPrintf("======================\n");
        break;
    }
    case PDU_TYPE_DATA: {
        DataPDU DP = BtoDataPDU(buf);
        Data D = contPTD(&DP);
        debugPrintf("======================\n");
        debugPrintf("[ DataPDU ]\n");
        debugPrintf("学号:   %s\n", D.sid);
        debugPrintf("课程号: %s\n", D.CourseNumber);
        debugPrintf("学期:   %s\n", D.P);
        debugPrintf("状态:   %s\n", D.status);
        debugPrintf("教工号: %s\n", D.tid);
        debugPrintf("成绩类型: %s\n", D.score.c_type);
        switch (D.score.type) {
        case 1:
            debugPrintf("成绩: %u\n", D.score.s.percentile);
            break;
        case 2:
            debugPrintf("成绩: %u\n", D.score.s.fivePoint);
            break;
        case 3:
            debugPrintf("成绩: %s\n", D.score.s.examCheck);
            break;
        }
        debugPrintf("======================\n");
        break;
    }
    default:
        debugPrintf("======================\n");
        debugPrintf("未知数据包类型: 0x%02X\n", type);
        debugPrintf("======================\n");
        break;
    }
}


void server_process(int fd, struct sockaddr* addr) {
    int n;
    int len;
    char tmp_buff[LENGTH];
    while (1)
    {
        len = sizeof(*addr);
        n = recvfrom(fd , tmp_buff , LENGTH, 0 , addr, &len);
        ProcessPDU(tmp_buff);
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