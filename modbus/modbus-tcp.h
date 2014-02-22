//------------------------------------------------------------------------------
// Copyright (C) 2010-2012, Robert Johansson <rob@raditex.nu> Raditex Control AB
// All rights reserved.
//
// rSCADA 
// http://www.rSCADA.se
// info@rscada.se
//
//------------------------------------------------------------------------------

#ifndef _MODBUS_TCP_H_
#define _MODBUS_TCP_H_

#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "modbus.h"


typedef struct __modbus_tcp_handle {
    
    struct hostent *addr;
    struct sockaddr_in saddr;

    //char * host;
    int port;

    int sock;

    int tcp_mode;
   
} modbus_tcp_handle_t;


int modbus_tcp_init(char *host, int port, modbus_tcp_handle_t *handle);

int modbus_tcp_close(modbus_tcp_handle_t *handle);

int modbus_tcp_send(modbus_tcp_handle_t *handle, modbus_frame_t *pkt);

int modbus_tcp_recv(modbus_tcp_handle_t *handle, modbus_frame_t *pkt);

int modbus_tcp_server_listen(modbus_tcp_handle_t *handle);
int modbus_tcp_server_init(int port, modbus_tcp_handle_t *handle);

#endif /* _MODBUS_tcp_H_ */
