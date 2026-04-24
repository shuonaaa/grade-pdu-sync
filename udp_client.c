#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include "PDULib/controlPDU.h"
#include "PDULib/dataPDU.h"

#define PORT_SERV 8888
#define LENGTH 32

static void input_control(char* buf) {
    Control c;
    int sel;

    printf("学号: ");
    scanf("%s", c.sid);

    printf("课程号: ");
    scanf("%s", c.CourseNumber);

    printf("学期 (1=上学期, 2=下学期): ");
    scanf("%d", &sel);
    if      (sel == 1) snprintf(c.P, sizeof(c.P), "上学期");
    else if (sel == 2) snprintf(c.P, sizeof(c.P), "下学期");
    else               snprintf(c.P, sizeof(c.P), "未知");

    printf("状态:\n");
    printf("  1. 成绩已提交阶段\n");
    printf("  2. 学院成绩检查通过\n");
    printf("  3. 学院成绩确认\n");
    printf("  4. 对象成绩结束\n");
    printf("  5. 异常\n");
    printf("请输入: ");
    scanf("%d", &sel);
    switch (sel) {
    case 1: snprintf(c.status, sizeof(c.status), "成绩已提交阶段");   break;
    case 2: snprintf(c.status, sizeof(c.status), "学院成绩检查通过"); break;
    case 3: snprintf(c.status, sizeof(c.status), "学院成绩确认");     break;
    case 4: snprintf(c.status, sizeof(c.status), "对象成绩结束");     break;
    case 5: snprintf(c.status, sizeof(c.status), "异常");             break;
    default: snprintf(c.status, sizeof(c.status), "特殊异常");        break;
    }

    ControlPDU pdu = contCTP(&c);
    pdu.type = PDU_TYPE_CONTROL;
    ControlPDUtoB(&pdu, buf);
}

static void input_data(char* buf) {
    Data d;
    int sel;

    printf("学号: ");
    scanf("%s", d.sid);

    printf("课程号: ");
    scanf("%s", d.CourseNumber);

    printf("学期 (1=上学期, 2=下学期): ");
    scanf("%d", &sel);
    if      (sel == 1) snprintf(d.P, sizeof(d.P), "上学期");
    else if (sel == 2) snprintf(d.P, sizeof(d.P), "下学期");
    else               snprintf(d.P, sizeof(d.P), "未知");

    printf("状态:\n");
    printf("  1. 成绩已提交阶段\n");
    printf("  2. 学院成绩检查通过\n");
    printf("  3. 学院成绩确认\n");
    printf("  4. 对象成绩结束\n");
    printf("  5. 异常\n");
    printf("请输入: ");
    scanf("%d", &sel);
    switch (sel) {
    case 1: snprintf(d.status, sizeof(d.status), "成绩已提交阶段");   break;
    case 2: snprintf(d.status, sizeof(d.status), "学院成绩检查通过"); break;
    case 3: snprintf(d.status, sizeof(d.status), "学院成绩确认");     break;
    case 4: snprintf(d.status, sizeof(d.status), "对象成绩结束");     break;
    case 5: snprintf(d.status, sizeof(d.status), "异常");             break;
    default: snprintf(d.status, sizeof(d.status), "特殊异常");        break;
    }

    printf("教工号: ");
    scanf("%s", d.tid);

    printf("成绩类型 (1=百分制, 2=五分制, 3=考查): ");
    scanf("%d", &sel);
    d.score.type = sel;
    switch (sel) {
    case 1:
        snprintf(d.score.c_type, sizeof(d.score.c_type), "百分制");
        printf("成绩 (0-100): ");
        scanf("%hhu", &d.score.s.percentile);
        break;
    case 2:
        snprintf(d.score.c_type, sizeof(d.score.c_type), "五分制");
        printf("成绩 (0-5): ");
        scanf("%hhu", &d.score.s.fivePoint);
        break;
    case 3:
        snprintf(d.score.c_type, sizeof(d.score.c_type), "考查");
        printf("成绩 (1=优, 2=良, 3=中, 4=合格, 5=不合格): ");
        scanf("%d", &sel);
        switch (sel) {
        case 1: snprintf(d.score.s.examCheck, sizeof(d.score.s.examCheck), "优");    break;
        case 2: snprintf(d.score.s.examCheck, sizeof(d.score.s.examCheck), "良");    break;
        case 3: snprintf(d.score.s.examCheck, sizeof(d.score.s.examCheck), "中");    break;
        case 4: snprintf(d.score.s.examCheck, sizeof(d.score.s.examCheck), "合格");  break;
        case 5: snprintf(d.score.s.examCheck, sizeof(d.score.s.examCheck), "不合格"); break;
        default: snprintf(d.score.s.examCheck, sizeof(d.score.s.examCheck), "未知"); break;
        }
        break;
    default:
        snprintf(d.score.c_type, sizeof(d.score.c_type), "未知");
        d.score.s.percentile = 0;
        break;
    }

    DataPDU pdu = contDTP(&d);
    pdu.type = PDU_TYPE_DATA;
    DataPDUtoB(&pdu, buf);
}

void scanf_in_tml(char* buf) {
    // ask PDU type
    int type = 0;
    printf("======================\n");
    printf("PDU 类型 (1=Control, 2=Data): "); 
    scanf("%d", &type);
    if(type == 1) {
        input_control(buf); 
    }
    else if(type == 2) {
        input_data(buf);
    }
    if(type == 1 || type == 2) {
        debugPrintHex(buf, LENGTH);
    }
}

void scanf_in_database(char* buf) {
    printf("to be finished\n");
}

void inputMessage(char* buf) {
    int type = 0;

    switch (type)
    {
    // 0: write in terminal
    case 0:
        scanf_in_tml(buf);
        break;
    // 1: get from database , if database(week_7_client) changed , send message
    case 1:
        scanf_in_database(buf);
        break;
    default:
        break;
    }
}

void client_process(int fd, struct sockaddr* addr) {
    int n;
    int len;
    char tmp_buff[LENGTH];
    while (1)
    {
        memset(tmp_buff, 0, LENGTH);
        inputMessage(tmp_buff);
        len = sizeof(*addr);
        n = sendto(fd , tmp_buff , LENGTH, 0 , addr, len);  
    }    
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