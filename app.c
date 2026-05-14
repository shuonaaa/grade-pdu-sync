#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mysql/mysql.h>
#include "PDULib/controlPDU.h"

// 检查分数合法

int validate_score(const char* type, int raw) {
    if      (strcmp(type, "percentile") == 0) return (raw >= 0 && raw <= 100);
    else if (strcmp(type, "fivePoint")  == 0) return (raw >= 0 && raw <= 5);
    else if (strcmp(type, "examCheck")  == 0) return (raw >= 1 && raw <= 5);
    return 0;
}

int examcheck_to_int(const char* s) {
    if (strcmp(s, "1")    == 0) return 1;
    if (strcmp(s, "2")    == 0) return 2;
    if (strcmp(s, "3")    == 0) return 3;
    if (strcmp(s, "4")    == 0) return 4;
    if (strcmp(s, "5")    == 0) return 5;
    if (strcmp(s, "优")   == 0) return 1;
    if (strcmp(s, "良")   == 0) return 2;
    if (strcmp(s, "中")   == 0) return 3;
    if (strcmp(s, "合格") == 0) return 4;
    if (strcmp(s, "差")   == 0) return 5;
    if (strcmp(s, "不合格") == 0) return 5;
    if (strcmp(s, "不及格") == 0) return 5;
    return -1;
}

// 状态编码

uint8_t status_to_code(const char* s) {
    if (!s)                           return 0xFF;
    if (strcmp(s, "pending")   == 0)  return 0;
    if (strcmp(s, "submitted") == 0)  return 1;
    if (strcmp(s, "checked")   == 0)  return 2;
    if (strcmp(s, "confirmed") == 0)  return 3;
    if (strcmp(s, "finished")  == 0)  return 4;
    if (strcmp(s, "exception") == 0)  return 5;
    return 0xFF;
}


const char* status_cn(const char* e) {
    if (!e) return "未知";
    if (strcmp(e,"pending")==0)   return "初始状态";
    if (strcmp(e,"submitted")==0) return "成绩已提交阶段";
    if (strcmp(e,"checked")==0)   return "学院成绩检查通过";
    if (strcmp(e,"confirmed")==0) return "学院成绩确认";
    if (strcmp(e,"finished")==0)  return "对象成绩结束";
    if (strcmp(e,"exception")==0) return "异常";
    return e;
}

// server DB 查询

int srv_score_type(MYSQL* srv, int courseNumber, const char* period, char* out, size_t n) {
    char q[256];
    snprintf(q, sizeof(q),
        "SELECT ScoreType FROM Course WHERE courseNumber=%d AND period='%s'",
        courseNumber, period);
    if (mysql_real_query(srv, q, strlen(q))) return -1;
    MYSQL_RES* r = mysql_store_result(srv); if (!r) return -1;
    MYSQL_ROW row = mysql_fetch_row(r);
    if (!row) { mysql_free_result(r); return -1; }
    strncpy(out, row[0], n-1); out[n-1]='\0';
    mysql_free_result(r);
    return 0;
}

int srv_sc_status(MYSQL* srv, int sid, int tid, int courseNumber, const char* period, char* out, size_t n) {
    char q[256];
    if (tid >= 0)
        snprintf(q, sizeof(q),
            "SELECT status FROM SC WHERE sid=%d AND tid=%d AND courseNumber=%d AND period='%s'",
            sid, tid, courseNumber, period);
    else
        snprintf(q, sizeof(q),
            "SELECT status FROM SC WHERE sid=%d AND courseNumber=%d AND period='%s' LIMIT 1",
            sid, courseNumber, period);
    if (mysql_real_query(srv, q, strlen(q))) return -1;
    MYSQL_RES* r = mysql_store_result(srv); if (!r) return -1;
    MYSQL_ROW row = mysql_fetch_row(r);
    if (!row || !row[0]) { mysql_free_result(r); return 0; }
    strncpy(out, row[0], n-1); out[n-1]='\0';
    mysql_free_result(r);
    return 1;
}

// client DB 写入 

void cli_write_data(MYSQL* cli,
                            int sid, int tid, int courseNumber,
                            const char* period, int raw, const char* status_enum) {
    char q[512];
    snprintf(q, sizeof(q),
        "SELECT COUNT(*) FROM SC WHERE sid=%d AND tid=%d AND courseNumber=%d AND period='%s'",
        sid, tid, courseNumber, period);
    if (mysql_real_query(cli, q, strlen(q))) {
        printf("  [错误] client 查询: %s\n", mysql_error(cli)); return;
    }
    MYSQL_RES* r = mysql_store_result(cli); if (!r) return;
    MYSQL_ROW row = mysql_fetch_row(r);
    int cnt = row ? atoi(row[0]) : 0;
    mysql_free_result(r);

    if (cnt == 0)
        snprintf(q, sizeof(q),
            "INSERT INTO SC (sid,tid,courseNumber,period,RealScore,status,sent)"
            " VALUES (%d,%d,%d,'%s',%d,'%s',1)",
            sid, tid, courseNumber, period, raw, status_enum);
    else
        snprintf(q, sizeof(q),
            "UPDATE SC SET RealScore=%d,status='%s',sent=1"
            " WHERE sid=%d AND tid=%d AND courseNumber=%d AND period='%s'",
            raw, status_enum, sid, tid, courseNumber, period);

    if (mysql_real_query(cli, q, strlen(q)))
        printf("  [错误] client 写入: %s\n", mysql_error(cli));
    else
        printf("  [排队] DataPDU sent=1  sid=%d tid=%d course=%d score=%d status=%s\n",
               sid, tid, courseNumber, raw, status_enum);
}

void cli_write_control(MYSQL* cli,
                               int sid, int courseNumber,
                               const char* period, const char* status_enum) {
    char q[256];
    snprintf(q, sizeof(q),
        "UPDATE SC SET status='%s',sent=2 WHERE sid=%d AND courseNumber=%d AND period='%s'",
        status_enum, sid, courseNumber, period);
    mysql_real_query(cli, q, strlen(q));
    if (mysql_affected_rows(cli) > 0)
        printf("  [排队] ControlPDU sent=2  sid=%d course=%d period=%s status=%s\n",
               sid, courseNumber, period, status_enum);
    else
        printf("  [错误] client 无记录 sid=%d course=%d period=%s，ControlPDU 需先提交成绩\n",
               sid, courseNumber, period);
}

// 业务校验

void do_data(MYSQL* cli, MYSQL* srv,
                    int sid, int tid, int courseNumber,
                    const char* period_enum, const char* score_str,
                    const char* status_enum) {

    // 输入状态只接受 pending / submitted
    uint8_t to_code = status_to_code(status_enum);
    if (to_code != 0 && to_code != 1) {
        printf("  [拒绝] DataPDU 状态必须为 pending 或 submitted，当前: %s\n", status_enum);
        return;
    }

    // 课程必须存在，获取成绩类型
    char score_type[32] = {0};
    if (srv_score_type(srv, courseNumber, period_enum, score_type, sizeof(score_type)) < 0) {
        printf("  [拒绝] server 上课程 %d period=%s 不存在\n", courseNumber, period_enum);
        return;
    }

    // 分数合法性
    int raw = (strcmp(score_type,"examCheck")==0) ? examcheck_to_int(score_str) : atoi(score_str);
    if (!validate_score(score_type, raw)) {
        printf("  [拒绝] 分数 '%s'(%d) 超出 %s 合法区间\n", score_str, raw, score_type);
        return;
    }

    // server 当前状态必须是 pending 或 submitted
    char srv_status[32] = "pending";
    int found = srv_sc_status(srv, sid, tid, courseNumber, period_enum, srv_status, sizeof(srv_status));
    if (found < 0) { printf("  [错误] server 查询失败\n"); return; }

    uint8_t srv_code = status_to_code(srv_status);
    if (found && srv_code != 0 && srv_code != 1) {
        printf("  [拒绝] server 当前状态 %s(%s)，只有 pending/submitted 可修改成绩\n",
               srv_status, status_cn(srv_status));
        return;
    }

    // server 已是 submitted 时，不接受 pending 回退
    if (found && srv_code == 1 && to_code == 0) {
        printf("  [拒绝] server 已是 submitted，不接受 pending 回退\n");
        return;
    }

    cli_write_data(cli, sid, tid, courseNumber, period_enum, raw, status_enum);
}

void do_control(MYSQL* cli, MYSQL* srv,int sid, int courseNumber,
                const char* period_enum, const char* status_enum) {

    // server 必须有记录
    char srv_status[32] = {0};
    int found = srv_sc_status(srv, sid, -1, courseNumber, period_enum, srv_status, sizeof(srv_status));
    if (found <= 0) {
        printf("  [拒绝] server 无 (sid=%d courseNumber=%d period=%s) 记录，不允许 ControlPDU\n",
               sid, courseNumber, period_enum);
        return;
    }

    // 状态转换合法性
    uint8_t from = status_to_code(srv_status);
    uint8_t to   = status_to_code(status_enum);
    if (!status_transition_valid(from, to)) {
        printf("  [拒绝] 非法状态转换: %s → %s\n",
               status_cn(srv_status), status_cn(status_enum));
        return;
    }

    cli_write_control(cli, sid, courseNumber, period_enum, status_enum);
}

// 查看失败记录

void view_failed(MYSQL* cli) {
    const char* q =
        "SELECT SC.sid, SC.tid, SC.courseNumber, SC.period, SC.RealScore, SC.status"
        " FROM SC WHERE SC.sent = 3";
    if (mysql_real_query(cli, q, strlen(q))) {
        printf("查询失败: %s\n", mysql_error(cli)); return;
    }
    MYSQL_RES* r = mysql_store_result(cli); if (!r) return;

    printf("\n%-12s %-10s %-14s %-8s %-6s %-20s\n",
           "学号","教工号","课程号","学期","分数","状态");
    printf("--------------------------------------------------------------\n");
    MYSQL_ROW row; int cnt = 0;
    while ((row = mysql_fetch_row(r))) {
        const char* period =
            (row[3] && strcmp(row[3],"last_per")==0) ? "上学期" :
            (row[3] && strcmp(row[3],"next_per")==0) ? "下学期" : "-";
        printf("%-12s %-10s %-14s %-8s %-6s %-20s\n",
               row[0]?row[0]:"-", row[1]?row[1]:"-", row[2]?row[2]:"-",
               period, row[4]?row[4]:"-", status_cn(row[5]));
        cnt++;
    }
    if (cnt == 0) printf("(无发送失败记录)\n");
    mysql_free_result(r);
}

// 交互式菜单

void interactive(MYSQL* cli, MYSQL* srv) {
    const char* ctl_codes[] =
        {"pending","submitted","checked","confirmed","finished","exception"};
    int choice;

    while (1) {
        printf("\n========================================\n");
        printf("  1. 提交/更新成绩   (DataPDU)\n");
        printf("  2. 发送状态控制    (ControlPDU)\n");
        printf("  3. 查看失败记录    (sent=3)\n");
        printf("  0. 退出\n");
        printf("========================================\n请选择: ");
        if (scanf("%d", &choice) != 1) break;
        if (choice == 0) break;

        if (choice == 1) {
            int sid, tid, courseNumber, period_sel, ssel;
            char score_str[32];

            printf("学号: ");             scanf("%d", &sid);
            printf("教工号: ");           scanf("%d", &tid);
            printf("课程号: ");           scanf("%d", &courseNumber);
            printf("学期 (1=上 2=下): "); scanf("%d", &period_sel);
            printf("状态 (1=pending 2=submitted): "); scanf("%d", &ssel);
            if (period_sel != 1 && period_sel != 2) {
                printf("  [错误] 无效学期\n");
                continue;
            }
            const char* period_enum  = (period_sel == 1) ? "last_per" : "next_per";
            const char* status_enum  = (ssel == 1) ? "pending" : "submitted";

            // 先查课程类型，给出对应提示
            char score_type[32] = {0};
            if (srv_score_type(srv, courseNumber, period_enum, score_type, sizeof(score_type)) < 0) {
                printf("  [拒绝] server 上课程 %d period=%s 不存在\n", courseNumber, period_enum);
                continue;
            }
            if      (strcmp(score_type,"examCheck")  == 0) printf("成绩 (1=优 2=良 3=中 4=合格 5=不合格/差): ");
            else if (strcmp(score_type,"percentile") == 0) printf("成绩 (0-100): ");
            else                                            printf("成绩 (0-5): ");
            scanf("%31s", score_str);

            do_data(cli, srv, sid, tid, courseNumber, period_enum, score_str, status_enum);

        } else if (choice == 2) {
            int sid, courseNumber, period_sel, ssel;
            printf("学号: ");             scanf("%d", &sid);
            printf("课程号: ");           scanf("%d", &courseNumber);
            printf("学期 (1=上 2=下): "); scanf("%d", &period_sel);
            if (period_sel != 1 && period_sel != 2) {
                printf("  [错误] 无效学期\n");
                continue;
            }
            const char* period_enum = (period_sel == 1) ? "last_per" : "next_per";

            printf("目标状态:\n");
            printf("  0. 初始状态       (pending)    [仅异常后重置]\n");
            printf("  1. 成绩已提交     (submitted)  [退回修改]\n");
            printf("  2. 学院检查通过   (checked)\n");
            printf("  3. 学院成绩确认   (confirmed)\n");
            printf("  4. 对象成绩结束   (finished)\n");
            printf("  5. 异常           (exception)\n");
            printf("请选择: "); scanf("%d", &ssel);
            if (ssel < 0 || ssel > 5) { printf("  [错误] 无效选项\n"); continue; }

            do_control(cli, srv, sid, courseNumber, period_enum, ctl_codes[ssel]);

        } else if (choice == 3) {
            view_failed(cli);
        }
    }
}

int main(void) {
    MYSQL* cli = mysql_init(NULL);
    MYSQL* srv = mysql_init(NULL);

    if (!mysql_real_connect(cli,"127.0.0.1","root","123456","week_7_client",0,NULL,0)) {
        printf("连接 week_7_client 失败: %s\n", mysql_error(cli)); return 1;
    }
    if (!mysql_real_connect(srv,"127.0.0.1","root","123456","week_7_server",0,NULL,0)) {
        printf("连接 week_7_server 失败: %s\n", mysql_error(srv)); return 1;
    }
    mysql_set_character_set(cli, "utf8");
    mysql_set_character_set(srv, "utf8");

    interactive(cli, srv);

    mysql_close(cli);
    mysql_close(srv);
    return 0;
}
