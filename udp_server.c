#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <syslog.h>
#include <fcntl.h>
#include <sys/file.h>
#include <mysql/mysql.h>
#include "PDULib/controlPDU.h"
#include "PDULib/dataPDU.h"
#include <errno.h>

#define PORT_SERVER 8888
#define LENGTH 32

static uint8_t mysql_enum_to_status_code(const char* s) {
    if      (strcmp(s, "pending")   == 0) return 0;
    else if (strcmp(s, "submitted") == 0) return 1;
    else if (strcmp(s, "checked")   == 0) return 2;
    else if (strcmp(s, "confirmed") == 0) return 3;
    else if (strcmp(s, "finished")  == 0) return 4;
    else if (strcmp(s, "exception") == 0) return 5;
    else                                   return 0xFF;
}

static int validate_score(const char* score_type, int raw) {
    if      (strcmp(score_type, "percentile") == 0) return (raw >= 0 && raw <= 100);
    else if (strcmp(score_type, "fivePoint")  == 0) return (raw >= 0 && raw <= 5);
    else if (strcmp(score_type, "examCheck")  == 0) return (raw >= 1 && raw <= 5);
    return 0;
}

const char* status_to_enum(const char* status) {
    if      (strcmp(status, "初始状态")         == 0) return "pending";
    else if (strcmp(status, "成绩已提交阶段")   == 0) return "submitted";
    else if (strcmp(status, "学院成绩检查通过") == 0) return "checked";
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

    const char* pdu_score_type;
    switch (D->score.type) {
    case 1:  pdu_score_type = "percentile"; break;
    case 2:  pdu_score_type = "fivePoint";  break;
    default: pdu_score_type = "examCheck";  break;
    }

    char check[256];
    snprintf(check, sizeof(check),
        "SELECT C.ScoreType, SC.status FROM Course C "
        "LEFT JOIN SC ON SC.courseNumber = C.courseNumber "
        "  AND SC.sid = %d AND SC.tid = %d "
        "WHERE C.courseNumber = %d",
        sid, tid, courseNumber);

    if (mysql_real_query(conn, check, strlen(check))) {
        debugPrintf("insert_data_to_db 查询失败: %s\n", mysql_error(conn));
        return;
    }

    MYSQL_RES* res = mysql_store_result(conn);
    if (!res) return;
    MYSQL_ROW row = mysql_fetch_row(res);

    if (!row) {
        // Course 表里找不到该课程，数据异常
        debugPrintf("insert_data_to_db 跳过: courseNumber=%d 不存在于 Course 表\n", courseNumber);
        mysql_free_result(res);
        return;
    }

    const char* db_score_type = row[0];  
    const char* db_status     = row[1];   

    mysql_free_result(res);

    // 成绩类型是否一致
    if (strcmp(db_score_type, pdu_score_type) != 0) {
        debugPrintf("insert_data_to_db 跳过: 成绩类型不一致 (Course=%s, PDU=%s)\n",
                    db_score_type, pdu_score_type);
        return;
    }

    // 分数是否合法
    if (!validate_score(db_score_type, realScore)) {
        debugPrintf("insert_data_to_db 跳过: 分数 %d 超出 %s 合法区间\n",
                    realScore, db_score_type);
        return;
    }

    // PDU状态是否合法
    if (strcmp(status_str, "submitted") != 0 && strcmp(status_str, "pending") != 0) {
        debugPrintf("insert_data_to_db 跳过: PDU 状态非 submitted 或 pending (PDU=%s)\n", status_str);
        return;
    }

    char query[256];
    if (db_status == NULL) {
        // SC 无记录 : INSERT
        snprintf(query, sizeof(query),
            "INSERT INTO SC (sid, tid, courseNumber, period, RealScore, status) "
            "VALUES (%d, %d, %d, '%s', %d, '%s')",
            sid, tid, courseNumber, period_str, realScore, status_str);
        debugPrintf("发出(INSERT): %s\n", query);
        if (mysql_real_query(conn, query, strlen(query)))
            debugPrintf("insert_data_to_db INSERT 失败: %s\n", mysql_error(conn));
        else
            debugPrintf("insert_data_to_db INSERT 成功: sid=%s\n", D->sid);
    } else {
        // 状态是 submited 时 ， 不能设为pending
        if (strcmp(db_status, "submitted") == 0 && strcmp(status_str, "pending") == 0) {
            debugPrintf("insert_data_to_db 跳过: 不可以将 submited 状态改为 pending\n");
            return;
        }
        // SC 有记录 : 状态必须是 submitted 或 pending 才能更新
        if (strcmp(db_status, "submitted") != 0 && strcmp(db_status, "pending") != 0) {
            debugPrintf("insert_data_to_db 跳过: SC 状态非 submitted 或 pending (DB=%s)\n", db_status);
            return;
        }
        snprintf(query, sizeof(query),
            "UPDATE SC SET RealScore = %d , status='%s' "
            "WHERE sid = %d AND tid = %d AND courseNumber = %d",
            realScore, status_str, sid, tid, courseNumber);
        debugPrintf("发出(UPDATE): %s\n", query);
        if (mysql_real_query(conn, query, strlen(query)))
            debugPrintf("insert_data_to_db UPDATE 失败: %s\n", mysql_error(conn));
        else
            debugPrintf("insert_data_to_db UPDATE 成功: sid=%s\n", D->sid);
    }
}

void insert_control_to_db(MYSQL* conn, Control* C) {
    int sid          = atoi(C->sid);
    int courseNumber = atoi(C->CourseNumber);
    const char* status_str = status_to_enum(C->status);

    // 查当前状态以校验转换合法性
    char check[256];
    snprintf(check, sizeof(check),
        "SELECT status FROM SC WHERE sid = %d AND courseNumber = %d",
        sid, courseNumber);
    if (mysql_real_query(conn, check, strlen(check))) {
        debugPrintf("insert_control_to_db 查询失败: %s\n", mysql_error(conn));
        return;
    }
    MYSQL_RES* res = mysql_store_result(conn);
    if (!res) return;
    MYSQL_ROW row = mysql_fetch_row(res);
    if (!row) {
        debugPrintf("insert_control_to_db 跳过: sid=%d courseNumber=%d 无记录\n", sid, courseNumber);
        mysql_free_result(res);
        return;
    }
    const char* cur_status = row[0];
    if (!status_transition_valid(mysql_enum_to_status_code(cur_status),
                                 mysql_enum_to_status_code(status_str))) {
        debugPrintf("insert_control_to_db 跳过: 非法状态转换 (%s → %s)\n", cur_status, status_str);
        mysql_free_result(res);
        return;
    }
    mysql_free_result(res);

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
        syslog(LOG_ERR, "mysql_real_connect 失败: %s", mysql_error(conn));
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

int lockfile(int fd) {
    struct flock fl;
    fl.l_type = F_WRLCK;
    fl.l_start = 0;
    fl.l_whence = SEEK_SET;
    fl.l_len = 0;
    return(fcntl(fd,F_SETLK,&fl));
}

int main(int agrc , char* argv[]) {
    int fd_serv;
    struct sockaddr_in addr_serv , addr_clie;

    daemon(0, 0);
    openlog("udp_server", LOG_PID, LOG_DAEMON);

    int pid_fd = open("/var/run/udp_server.pid", O_RDWR | O_CREAT, 0755);
    if (pid_fd < 0) { syslog(LOG_ERR, "无法打开 PID 文件"); exit(1); }
    if (lockfile(pid_fd) < 0) {
        if(errno == EACCES || errno == EAGAIN) {
            close(pid_fd);
            syslog(LOG_ERR, "已有实例在运行，退出");
            exit(1);
        } 
        syslog(LOG_ERR, "已有实例在运行，退出");
        exit(1);
    }
    ftruncate(pid_fd,0);
    char pid_str[16];
    snprintf(pid_str, sizeof(pid_str), "%d\n", getpid());
    write(pid_fd, pid_str, strlen(pid_str));

    fd_serv = socket(AF_INET,SOCK_DGRAM,0);

    memset(&addr_serv, 0 , sizeof(addr_serv));
    addr_serv.sin_family = AF_INET;
    inet_aton("127.0.0.1", &addr_serv.sin_addr);
    syslog(LOG_INFO, "listening on %s:%d", inet_ntoa(addr_serv.sin_addr), PORT_SERVER);
    addr_serv.sin_port = htons(PORT_SERVER);

    bind(fd_serv , (struct sockaddr*)&addr_serv , sizeof(addr_serv));
    server_process(fd_serv , (struct sockaddr*)&addr_clie);

    closelog();
    exit(0);
}