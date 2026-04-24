#include "controlPDU.h"
#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>

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

    if(strcmp(c->status,"成绩已提交阶段")==0)
        pdu.status = 1;
    else if(strcmp(c->status,"学院成绩检查通过")==0)
        pdu.status = 2;
    else if (strcmp(c->status,"学院成绩确认")==0)
        pdu.status = 3;
    else if (strcmp(c->status,"对象成绩结束")==0) 
        pdu.status = 4;
    else if (strcmp(c->status,"异常")==0) 
        pdu.status = 5;
    else
        pdu.status = 0;

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