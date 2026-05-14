CC := gcc
CFLAGS := -Wall -Wextra -I.
LDLIBS := -lmysqlclient

PDU_SRCS := PDULib/controlPDU.c PDULib/dataPDU.c

.PHONY: all debug clean

all: client server app

client: udp_client.c $(PDU_SRCS) PDULib/controlPDU.h PDULib/dataPDU.h
	$(CC) $(CFLAGS) udp_client.c $(PDU_SRCS) -o $@ $(LDLIBS)

server: udp_server.c $(PDU_SRCS) PDULib/controlPDU.h PDULib/dataPDU.h
	$(CC) $(CFLAGS) udp_server.c $(PDU_SRCS) -o $@ $(LDLIBS)

app: app.c PDULib/controlPDU.c PDULib/controlPDU.h
	$(CC) $(CFLAGS) app.c PDULib/controlPDU.c -o $@ $(LDLIBS)

debug: client_debug server_debug app

client_debug: udp_client.c $(PDU_SRCS) PDULib/controlPDU.h PDULib/dataPDU.h
	$(CC) $(CFLAGS) -DDEBUG udp_client.c $(PDU_SRCS) -o $@ $(LDLIBS)

server_debug: udp_server.c $(PDU_SRCS) PDULib/controlPDU.h PDULib/dataPDU.h
	$(CC) $(CFLAGS) -DDEBUG udp_server.c $(PDU_SRCS) -o $@ $(LDLIBS)

clean:
	rm -f client server app client_debug server_debug
