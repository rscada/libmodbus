# ------------------------------------------------------------------------------
#  Copyright (C) 2010, Raditex AB
#  All rights reserved.
# 
#  rSCADA 
#  http://www.rSCADA.se
#  info@rscada.se
# 
# ------------------------------------------------------------------------------


CFLAGS=-Wall -g -I..
LDFLAGS=-L.. -lmodbus

all: modbus_test1 modbus_udp_client_test1 modbus_tcp_client_test1 modbus_tcp_server_test1

%.o: %.c
	$(CC) -c $(CFLAGS) $< -o $@

modbus_test1: modbus_test1.o
	gcc -o $@ $^ $(LDFLAGS) 

modbus_udp_client_test1: modbus_udp_client_test1.o
	gcc -o $@ $^ $(LDFLAGS) 

modbus_tcp_client_test1: modbus_tcp_client_test1.o
	gcc -o $@ $^ $(LDFLAGS) 

modbus_tcp_server_test1: modbus_tcp_server_test1.o
	gcc -o $@ $^ $(LDFLAGS) 

clean:
	rm -rf *.o modbus_test1 modbus_test2 modbus_udp_client_test1 \
			modbus_tcp_client_test1 modbus_tcp_server_test1
