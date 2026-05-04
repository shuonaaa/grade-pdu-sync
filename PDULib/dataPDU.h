#ifndef DATA_PDU_H
  #define DATA_PDU_H          
  
  #include <syslog.h>

  #ifdef DEBUG
    #ifndef DEBUGPRINTF
      #define DEBUGPRINTF
      #define debugPrintf(fmt, ...) syslog(LOG_DEBUG, "[%s:%d] " fmt, __FILE__, __LINE__, ##__VA_ARGS__)
      #define debugPrintHex(buf, len) do { \
          char _hbuf[256] = {0}; int _i, _pos = 0; \
          for (_i = 0; _i < (len) && _pos + 3 < 256; _i++) \
              _pos += snprintf(_hbuf + _pos, 256 - _pos, "%02X ", (unsigned char)(buf)[_i]); \
          syslog(LOG_DEBUG, "[%s:%d] hex: %s", __FILE__, __LINE__, _hbuf); \
      } while(0)
    #endif
  #else
    #ifndef DEBUGPRINTF
      #define DEBUGPRINTF
      #define debugPrintf(fmt, ...)
      #define debugPrintHex(buf, len)
    #endif
  #endif
                  
  #include <stdint.h>                                                           
  #include <string.h>

  #define PDU_TYPE_DATA     0x02
                                                                                
  typedef struct __attribute__((packed)) {
      uint8_t   type;
      uint32_t  sid;
      uint32_t  CourseNumber;
      uint8_t   P;
      uint8_t   status;
      uint16_t  unused;
      uint32_t  tid;
      uint8_t   real_score_type;
      uint8_t   real_score_length;
      uint8_t   real_score_value;
  } DataPDU;

  typedef union 
  {
    uint8_t percentile;
    uint8_t fivePoint;
    char examCheck[16];
  } RealScore;

  typedef struct 
  {
    int type;
    char c_type[16];
    RealScore s;
  } Score;
    
  typedef struct 
  {
    char  sid[16];
    char  CourseNumber[16];
    char  P[16];
    char  status[32];
    char  tid[16];
    Score score;
  } Data;

  #define SCORE_TYPE_CLASS(t)   (((t) >> 6) & 0x03)                             
  #define SCORE_TYPE_PC(t)      (((t) >> 5) & 0x01)
  #define SCORE_TYPE_NUMBER(t)  ((t) & 0x1F)                                                         

  Data contPTD(const DataPDU* pdu);
  DataPDU contDTP(const Data* c);
  DataPDU BtoDataPDU(const char* buf);
  void DataPDUtoB(const DataPDU* pdu, char* buf);
#endif
