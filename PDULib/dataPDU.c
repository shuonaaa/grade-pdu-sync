#include "dataPDU.h"
#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>

Data contPTD(const DataPDU* pdu) {
    Data d;

    snprintf(d.sid,          sizeof(d.sid),          "%u", pdu->sid);
    snprintf(d.CourseNumber, sizeof(d.CourseNumber),  "%u", pdu->CourseNumber);
    snprintf(d.tid,          sizeof(d.tid),           "%u", pdu->tid);

    switch (pdu->P) {
    case 1:  
        snprintf(d.P, sizeof(d.P), "上学期"); 
        break;
    case 2:  
        snprintf(d.P, sizeof(d.P), "下学期"); 
        break;
    default: 
        snprintf(d.P, sizeof(d.P), "未知");   
        break;
    }

    switch (pdu->status) {
        case 1:  
            snprintf(d.status, sizeof(d.status), "成绩已提交阶段");   
            break;
        case 2:  
            snprintf(d.status, sizeof(d.status), "学院成绩检查通过"); 
            break;
        case 3:  
            snprintf(d.status, sizeof(d.status), "学院成绩确认");     
            break;
        case 4:  
            snprintf(d.status, sizeof(d.status), "对象成绩结束");     
            break;
        case 5:  
            snprintf(d.status, sizeof(d.status), "异常");             
            break;
        default: 
            snprintf(d.status, sizeof(d.status), "特殊异常");         
            break;
    }

    uint8_t score_class = SCORE_TYPE_CLASS(pdu->real_score_type);
    d.score.type = score_class;

    switch (score_class) {
    case 1:
        snprintf(d.score.c_type, sizeof(d.score.c_type), "百分制");
        d.score.s.percentile = pdu->real_score_value;
        break;
    case 2:
        snprintf(d.score.c_type, sizeof(d.score.c_type), "五分制");
        d.score.s.fivePoint = pdu->real_score_value;
        break;
    case 3:
        snprintf(d.score.c_type, sizeof(d.score.c_type), "考查");
        switch (pdu->real_score_value) {
        case 1:  
            snprintf(d.score.s.examCheck, sizeof(d.score.s.examCheck), "优");   
            break;
        case 2:  
            snprintf(d.score.s.examCheck, sizeof(d.score.s.examCheck), "良");   
            break;
        case 3:  
            snprintf(d.score.s.examCheck, sizeof(d.score.s.examCheck), "中");   
            break;
        case 4:  
            snprintf(d.score.s.examCheck, sizeof(d.score.s.examCheck), "合格"); 
            break;
        case 5:  
            snprintf(d.score.s.examCheck, sizeof(d.score.s.examCheck), "不合格"); 
            break;
        default: 
            snprintf(d.score.s.examCheck, sizeof(d.score.s.examCheck), "未知"); 
            break;
        }
        break;
    default:
        snprintf(d.score.c_type, sizeof(d.score.c_type), "未知");
        d.score.s.percentile = pdu->real_score_value;
        break;
    }

    return d;
}

DataPDU contDTP(const Data* d) {
    DataPDU pdu;
    memset(&pdu, 0, sizeof(DataPDU));

    pdu.sid          = (uint32_t)strtoul(d->sid,          NULL, 10);
    pdu.CourseNumber = (uint32_t)strtoul(d->CourseNumber,  NULL, 10);
    pdu.tid          = (uint32_t)strtoul(d->tid,           NULL, 10);

    if(strcmp(d->P, "上学期") == 0) 
        pdu.P = 1;
    else if(strcmp(d->P, "下学期") == 0) 
        pdu.P = 2;
    else
        pdu.P = 0;

    if(strcmp(d->status, "成绩已提交阶段") == 0) 
        pdu.status = 1;
    else if (strcmp(d->status, "学院成绩检查通过")==0) 
        pdu.status = 2;
    else if (strcmp(d->status, "学院成绩确认") == 0) 
        pdu.status = 3;
    else if (strcmp(d->status, "对象成绩结束") == 0) 
        pdu.status = 4;
    else if (strcmp(d->status, "异常") == 0) 
        pdu.status = 5;
    else 
        pdu.status = 0;

    pdu.real_score_type = (uint8_t)(d->score.type << 6);
    pdu.real_score_length = 1;

    switch (d->score.type) {
    case 1:
        pdu.real_score_value = d->score.s.percentile;
        break;
    case 2:
        pdu.real_score_value = d->score.s.fivePoint;
        break;
    case 3:
        if      (strcmp(d->score.s.examCheck, "优")   == 0) pdu.real_score_value = 1;
        else if (strcmp(d->score.s.examCheck, "良")   == 0) pdu.real_score_value = 2;
        else if (strcmp(d->score.s.examCheck, "中")   == 0) pdu.real_score_value = 3;
        else if (strcmp(d->score.s.examCheck, "合格") == 0) pdu.real_score_value = 4;
        else if (strcmp(d->score.s.examCheck, "不合格") == 0) pdu.real_score_value = 5;
        else                                                  pdu.real_score_value = 0;
        break;
    default:
        pdu.real_score_value = 0;
        break;
    }

    return pdu;
}

DataPDU BtoDataPDU(const char* buf) {
    DataPDU pdu;
    memcpy(&pdu, buf, sizeof(DataPDU));
    pdu.sid          = ntohl(pdu.sid);
    pdu.CourseNumber = ntohl(pdu.CourseNumber);
    pdu.tid          = ntohl(pdu.tid);
    return pdu;
}

void DataPDUtoB(const DataPDU* pdu, char* buf) {
    DataPDU tmp      = *pdu;
    tmp.sid          = htonl(pdu->sid);
    tmp.CourseNumber = htonl(pdu->CourseNumber);
    tmp.tid          = htonl(pdu->tid);
    memcpy(buf, &tmp, sizeof(DataPDU));
}
