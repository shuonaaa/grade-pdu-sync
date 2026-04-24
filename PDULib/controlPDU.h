#ifndef CONTROL_PDU_H
  #define CONTROL_PDU_H  
  
  #ifdef DEBUG
    #ifndef DEBUGPRINTF
      #define DEBUGPRINTF
      #define debugPrintf(fmt, ...) printf("[%s:%d] " fmt, __FILE__, __LINE__, ##__VA_ARGS__)
      #define debugPrintHex(buf, len) do { \
          printf("[%s:%d] ", __FILE__, __LINE__); \
          for (int _i = 0; _i < (len); _i++) \
              printf("%02X ", (unsigned char)(buf)[_i]); \
          printf("\n"); \
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

#endif
