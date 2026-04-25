#include "controlPDU.h"
#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>

/* 状态转换合法性表 [from][to], 1=允许, 0=禁止 */
static const uint8_t transition_table[6][6] = {
    /*             to: 0  1  2  3  4  5 */
    /* 0 pending    */{1, 1, 0, 0, 0, 0},
    /* 1 submitted  */{0, 1, 1, 0, 0, 1},
    /* 2 checked    */{0, 1, 1, 1, 0, 1},
    /* 3 confirmed  */{0, 0, 0, 1, 1, 1},
    /* 4 finished   */{0, 0, 0, 0, 1, 0},
    /* 5 exception  */{1, 0, 0, 0, 0, 1},
};

int status_transition_valid(uint8_t from, uint8_t to) {
    if (from > 5 || to > 5) return 0;
    return transition_table[from][to];
}

static uint8_t cn_to_status_code(const char* s) {
    if      (strcmp(s, "初始状态")         == 0) return 0;
    else if (strcmp(s, "成绩已提交阶段")   == 0) return 1;
    else if (strcmp(s, "学院成绩检查通过") == 0) return 2;
    else if (strcmp(s, "学院成绩确认")     == 0) return 3;
    else if (strcmp(s, "对象成绩结束")     == 0) return 4;
    else if (strcmp(s, "异常")             == 0) return 5;
    else                                          return 0xFF;
}

int status_transition_valid_cn(const char* from_cn, const char* to_cn) {
    return status_transition_valid(cn_to_status_code(from_cn),
                                   cn_to_status_code(to_cn));
}

Control contPTC(const ControlPDU* pdu) {
    Control c;
    switch (pdu->P)
    {
    case 1:
        snprintf(c.P, sizeof(c.P), "上学期");
        break;
    case 2:
        snprintf(c.P, sizeof(c.P), "下学期");
        break;
    default:
        snprintf(c.P, sizeof(c.P), "未知");
        break;
    }
    
    switch (pdu->status)
    {
    case 0:
        snprintf(c.status, sizeof(c.status), "初始状态");
        break;
    case 1:
        snprintf(c.status, sizeof(c.status), "成绩已提交阶段");
        break;
    case 2:
        snprintf(c.status, sizeof(c.status), "学院成绩检查通过");
        break;
    case 3:
        snprintf(c.status, sizeof(c.status), "学院成绩确认");
        break;
    case 4:
        snprintf(c.status, sizeof(c.status), "对象成绩结束");
        break;
    case 5:
        snprintf(c.status, sizeof(c.status), "异常");
        break;
    default:
        snprintf(c.status, sizeof(c.status), "特殊异常");
        break;
    }

    snprintf(c.CourseNumber, sizeof(c.CourseNumber), "%u", pdu->CourseNumber);
    snprintf(c.sid,sizeof(c.sid),"%u",pdu->sid);

    return c;
}

ControlPDU contCTP(const Control* c) {
    ControlPDU pdu;

    pdu.sid = (uint32_t)strtoul(c->sid, NULL, 10);
    pdu.CourseNumber = (uint32_t)strtoul(c->CourseNumber, NULL, 10);

    if(strcmp(c->P, "上学期") == 0)
        pdu.P = 1;
    else if(strcmp(c->P, "下学期") == 0)
        pdu.P = 2;
    else
        pdu.P = 0;

    if      (strcmp(c->status, "初始状态")         == 0) pdu.status = 0;
    else if (strcmp(c->status, "成绩已提交阶段")   == 0) pdu.status = 1;
    else if (strcmp(c->status, "学院成绩检查通过") == 0) pdu.status = 2;
    else if (strcmp(c->status, "学院成绩确认")     == 0) pdu.status = 3;
    else if (strcmp(c->status, "对象成绩结束")     == 0) pdu.status = 4;
    else if (strcmp(c->status, "异常")             == 0) pdu.status = 5;
    else                                                  pdu.status = 0xFF;

    return pdu;
}

ControlPDU BtoControlPDU(const char* buf) {
    ControlPDU pdu;
    memcpy(&pdu, buf, sizeof(ControlPDU));
    pdu.sid          = ntohl(pdu.sid);
    pdu.CourseNumber = ntohl(pdu.CourseNumber);
    return pdu;
}

void ControlPDUtoB(const ControlPDU* pdu, char* buf) {
    ControlPDU tmp    = *pdu;
    tmp.sid           = htonl(pdu->sid);
    tmp.CourseNumber  = htonl(pdu->CourseNumber);
    memcpy(buf, &tmp, sizeof(ControlPDU));
}