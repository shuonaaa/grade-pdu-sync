#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <mysql/mysql.h>
#include "PDULib/controlPDU.h"
#include "PDULib/dataPDU.h"

#define PORT_SERV 8888
#define LENGTH 32

int validate_score(const char* score_type, int raw) {
    if      (strcmp(score_type, "percentile") == 0) return (raw >= 0  && raw <= 100);
    else if (strcmp(score_type, "fivePoint")  == 0) return (raw >= 0  && raw <= 5);
    else if (strcmp(score_type, "examCheck")  == 0) return (raw >= 1  && raw <= 5);
    return 0;
}

void input_control(char* buf) {
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

void input_data(char* buf) {
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

void scanf_in_database(char* buf, MYSQL* conn) {
    // sent=1: 待发 DataPDU  sent=2: 待发 ControlPDU
    const char* query =
        "SELECT SC.sid, SC.tid, SC.courseNumber, SC.period, SC.RealScore, SC.status, C.ScoreType, SC.sent "
        "FROM SC "
        "JOIN Course C ON SC.courseNumber = C.courseNumber "
        "WHERE SC.sent IN (1, 2) LIMIT 1";

    if (mysql_real_query(conn, query, strlen(query))) {
        debugPrintf("scanf_in_database 查询失败: %s\n", mysql_error(conn));
        return;
    }

    MYSQL_RES* res = mysql_store_result(conn);
    if (!res) return;

    MYSQL_ROW row = mysql_fetch_row(res);
    if (!row) {
        debugPrintf("暂无待发数据，休眠 30s\n");
        mysql_free_result(res);
        sleep(30);
        return;
    }

    /* row: 0=sid 1=tid 2=courseNumber 3=period 4=RealScore 5=status 6=ScoreType 7=sent */
    int sent_flag = atoi(row[7]);
    char update[128];

    if (sent_flag == 1) {
        // DataPDU 成绩提交业务
        Data d;
        snprintf(d.sid,          sizeof(d.sid),          "%s", row[0]);
        snprintf(d.tid,          sizeof(d.tid),           "%s", row[1]);
        snprintf(d.CourseNumber, sizeof(d.CourseNumber),  "%s", row[2]);

        if      (strcmp(row[3], "last_per") == 0) snprintf(d.P, sizeof(d.P), "上学期");
        else if (strcmp(row[3], "next_per") == 0) snprintf(d.P, sizeof(d.P), "下学期");
        else                                       snprintf(d.P, sizeof(d.P), "未知");

        if      (strcmp(row[5], "pending")   == 0) snprintf(d.status, sizeof(d.status), "初始状态");
        else if (strcmp(row[5], "submitted") == 0) snprintf(d.status, sizeof(d.status), "成绩已提交阶段");
        else if (strcmp(row[5], "checked")   == 0) snprintf(d.status, sizeof(d.status), "学院成绩检查通过");
        else if (strcmp(row[5], "confirmed") == 0) snprintf(d.status, sizeof(d.status), "学院成绩确认");
        else if (strcmp(row[5], "finished")  == 0) snprintf(d.status, sizeof(d.status), "对象成绩结束");
        else if (strcmp(row[5], "exception") == 0) snprintf(d.status, sizeof(d.status), "异常");
        else                                        snprintf(d.status, sizeof(d.status), "特殊异常");

        int raw = atoi(row[4]);
        const char* course_score_type = row[6];

        if (strcmp(course_score_type, "percentile") == 0) {
            d.score.type = 1;
            snprintf(d.score.c_type, sizeof(d.score.c_type), "百分制");
            d.score.s.percentile = (uint8_t)raw;
        } else if (strcmp(course_score_type, "fivePoint") == 0) {
            d.score.type = 2;
            snprintf(d.score.c_type, sizeof(d.score.c_type), "五分制");
            d.score.s.fivePoint = (uint8_t)raw;
        } else {
            d.score.type = 3;
            snprintf(d.score.c_type, sizeof(d.score.c_type), "考查");
            switch (raw) {
            case 1:  snprintf(d.score.s.examCheck, sizeof(d.score.s.examCheck), "优");     break;
            case 2:  snprintf(d.score.s.examCheck, sizeof(d.score.s.examCheck), "良");     break;
            case 3:  snprintf(d.score.s.examCheck, sizeof(d.score.s.examCheck), "中");     break;
            case 4:  snprintf(d.score.s.examCheck, sizeof(d.score.s.examCheck), "合格");   break;
            default: snprintf(d.score.s.examCheck, sizeof(d.score.s.examCheck), "不合格"); break;
            }
        }

        int status_ok = (strcmp(d.status, "成绩已提交阶段") == 0);
        int score_ok  = validate_score(course_score_type, raw);
        if (!status_ok || !score_ok) {
            debugPrintf("DataPDU 跳过，标记 sent=3 (status=%s, ScoreType=%s, raw=%d)\n",
                        d.status, course_score_type, raw);
            snprintf(update, sizeof(update),
                "UPDATE SC SET sent=3 WHERE sid=%s AND tid=%s AND courseNumber=%s",
                row[0], row[1], row[2]);
            mysql_real_query(conn, update, strlen(update));
            mysql_free_result(res);
            return;
        }

        DataPDU pdu = contDTP(&d);
        pdu.type = PDU_TYPE_DATA;
        DataPDUtoB(&pdu, buf);
        debugPrintf("已打包(DataPDU): sid=%s courseNumber=%s\n", d.sid, d.CourseNumber);

        snprintf(update, sizeof(update),
            "UPDATE SC SET sent=0 WHERE sid=%s AND tid=%s AND courseNumber=%s",
            row[0], row[1], row[2]);
        mysql_real_query(conn, update, strlen(update));

    } else {
        // ControlPDU 状态控制
        Control c;
        snprintf(c.sid,          sizeof(c.sid),          "%s", row[0]);
        snprintf(c.CourseNumber, sizeof(c.CourseNumber),  "%s", row[2]);

        if      (strcmp(row[3], "last_per") == 0) snprintf(c.P, sizeof(c.P), "上学期");
        else if (strcmp(row[3], "next_per") == 0) snprintf(c.P, sizeof(c.P), "下学期");
        else                                       snprintf(c.P, sizeof(c.P), "未知");

        int status_ok = 1;
        if      (strcmp(row[5], "pending")   == 0) snprintf(c.status, sizeof(c.status), "初始状态");
        else if (strcmp(row[5], "submitted") == 0) snprintf(c.status, sizeof(c.status), "成绩已提交阶段");
        else if (strcmp(row[5], "checked")   == 0) snprintf(c.status, sizeof(c.status), "学院成绩检查通过");
        else if (strcmp(row[5], "confirmed") == 0) snprintf(c.status, sizeof(c.status), "学院成绩确认");
        else if (strcmp(row[5], "finished")  == 0) snprintf(c.status, sizeof(c.status), "对象成绩结束");
        else if (strcmp(row[5], "exception") == 0) snprintf(c.status, sizeof(c.status), "异常");
        else                                        status_ok = 0;

        if (!status_ok) {
            debugPrintf("ControlPDU 跳过: 非法控制状态 %s，标记 sent=3\n", row[5]);
            snprintf(update, sizeof(update),
                "UPDATE SC SET sent=3 WHERE sid=%s AND courseNumber=%s",
                row[0], row[2]);
            mysql_real_query(conn, update, strlen(update));
            mysql_free_result(res);
            return;
        }

        ControlPDU pdu = contCTP(&c);
        pdu.type = PDU_TYPE_CONTROL;
        ControlPDUtoB(&pdu, buf);
        debugPrintf("已打包(ControlPDU): sid=%s courseNumber=%s status=%s\n",
                    c.sid, c.CourseNumber, c.status);

        snprintf(update, sizeof(update),
            "UPDATE SC SET sent=0 WHERE sid=%s AND courseNumber=%s",
            row[0], row[2]);
        mysql_real_query(conn, update, strlen(update));
    }

    mysql_free_result(res);
}

void inputMessage(char* buf , MYSQL* conn) {
    int type = 1;

    switch (type)
    {
    // 0: write in terminal
    case 0:
        scanf_in_tml(buf);
        break;
    // 1: get from database , if database(week_7_client) changed , send message
    case 1:
        scanf_in_database(buf , conn);
        break;
    default:
        break;
    }
}

void client_process(int fd, struct sockaddr* addr) {
    int n;
    int len;
    char tmp_buff[LENGTH];

    MYSQL* conn = mysql_init(NULL);
    if (!mysql_real_connect(conn, "127.0.0.1", "root", "123456", "week_7_client", 0, NULL, 0)) {
      printf("mysql_real_connect 失败: %s\n", mysql_error(conn));                                                              
      exit(-1);
    }
    mysql_set_character_set(conn, "utf8");

    while (1)
    {
        memset(tmp_buff, 0, LENGTH);
        inputMessage(tmp_buff , conn);
        len = sizeof(*addr);
        uint8_t type = (uint8_t)tmp_buff[0];
        if (type == PDU_TYPE_CONTROL || type == PDU_TYPE_DATA) {
            n = sendto(fd, tmp_buff, LENGTH, 0, addr, len);
        }
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