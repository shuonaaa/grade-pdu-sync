#ifndef CONTROL_PDU_H
  #define CONTROL_PDU_H  
  
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
                                                                                
  #define PDU_TYPE_CONTROL  0x01

  typedef struct __attribute__((packed)) {
      uint8_t  type;
      uint32_t sid;
      uint32_t CourseNumber;
      uint8_t  P;
      uint8_t  status;
  } ControlPDU;
  
  typedef struct {
        char  sid[16];
        char  CourseNumber[16];
        char  P[16];
        char  status[32]; 
  } Control;
  Control contPTC(const ControlPDU* pdu);
  ControlPDU contCTP(const Control* c);
  ControlPDU BtoControlPDU(const char* buf);
  void ControlPDUtoB(const ControlPDU* pdu, char* buf);

  /* 状态机转换校验：1=合法, 0=非法 */
  int status_transition_valid(uint8_t from, uint8_t to);
  int status_transition_valid_cn(const char* from_cn, const char* to_cn);

#endif
