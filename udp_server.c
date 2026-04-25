#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <mysql/mysql.h>
#include "PDULib/controlPDU.h"
#include "PDULib/dataPDU.h"

#define PORT_SERVER 8888
#define LENGTH 32

const char* status_to_enum(const char* status) {
    if      (strcmp(status, "成绩已提交阶段")   == 0) return "submitted";
    else if (strcmp(status, "学院成绩检查通过") == 0) return "submitted";
    else if (strcmp(status, "学院成绩确认")     == 0) return "confirmed";
    else if (strcmp(status, "对象成绩结束")     == 0) return "finished";
    else if (strcmp(status, "异常")             == 0) return "exception";
    else                                              return "error";
}

const char* period_to_enum(const char* period) {
    if      (strcmp(period, "上学期")   == 0)   return "last_per";
    else if (strcmp(period, "下学期")   == 0)   return "next_per";
    else                                       return "error";
}

void insert_data_to_db(MYSQL* conn, Data* D) {
    int sid          = atoi(D->sid);
    int tid          = atoi(D->tid);
    int courseNumber = atoi(D->CourseNumber);
    const char* period_str = period_to_enum(D->P);
    const char* status_str = status_to_enum(D->status);

    int realScore;
    switch (D->score.type) {
    case 1: realScore = D->score.s.percentile; break;
    case 2: realScore = D->score.s.fivePoint;  break;
    case 3:
        if      (strcmp(D->score.s.examCheck, "优")    == 0) realScore = 1;
        else if (strcmp(D->score.s.examCheck, "良")    == 0) realScore = 2;
        else if (strcmp(D->score.s.examCheck, "中")    == 0) realScore = 3;
        else if (strcmp(D->score.s.examCheck, "合格")  == 0) realScore = 4;
        else                                                  realScore = 5;
        break;
    default: realScore = 0; break;
    }

    char query[256];
    snprintf(query, sizeof(query),
        "INSERT INTO SC (sid, tid, courseNumber, period, RealScore, status) VALUES (%d, %d, %d, '%s', %d, '%s')",
        sid, tid, courseNumber, period_str, realScore, status_str);
    debugPrintf("发出: %s\n", query);
    if (mysql_real_query(conn, query, strlen(query)))
        debugPrintf("insert_data_to_db 失败: %s\n", mysql_error(conn));
    else
        debugPrintf("insert_data_to_db 成功: sid=%s\n", D->sid);
}

void insert_control_to_db(MYSQL* conn, Control* C) {
    int sid          = atoi(C->sid);
    int courseNumber = atoi(C->CourseNumber);
    const char* status_str = status_to_enum(C->status);

    char query[128];
    snprintf(query, sizeof(query),
        "UPDATE SC SET status = '%s' WHERE sid = %d AND courseNumber = %d",
        status_str, sid, courseNumber);
    debugPrintf("发出: %s\n", query);
    if (mysql_real_query(conn, query, strlen(query)))
        debugPrintf("insert_control_to_db 失败: %s\n", mysql_error(conn));
    else
        debugPrintf("insert_control_to_db 成功: sid=%s\n", C->sid);
}

void ProcessPDU(char* buf, MYSQL* conn) {
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

        insert_control_to_db(conn, &C);

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

        insert_data_to_db(conn, &D);

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

    MYSQL* conn = mysql_init(NULL);
    if (!mysql_real_connect(conn, "127.0.0.1", "root", "123456", "week_7_server", 0, NULL, 0)) {
      printf("mysql_real_connect 失败: %s\n", mysql_error(conn));                                                              
      exit(-1);
    }
    mysql_set_character_set(conn, "utf8");
    while (1)
    {
        len = sizeof(*addr);
        n = recvfrom(fd , tmp_buff , LENGTH, 0 , addr, &len);
        ProcessPDU(tmp_buff , conn);
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