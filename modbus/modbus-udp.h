//------------------------------------------------------------------------------
// Copyright (C) 2010, Raditex AB
// All rights reserved.
//
// rSCADA 
// http://www.rSCADA.se
// info@rscada.se
//
//------------------------------------------------------------------------------

#ifndef _MODBUS_UDP_H_
#define _MODBUS_UDP_H_

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


typedef struct __modbus_udp_handle {
    
    struct hostent *addr;
    struct sockaddr_in saddr;

    //char * host;
    int port;

    int sock;

   
} modbus_udp_handle_t;


int modbus_udp_init(char *host, int port, modbus_udp_handle_t *handle);

int modbus_udp_close(modbus_udp_handle_t *handle);

int modbus_udp_send(modbus_udp_handle_t *handle, modbus_frame_t *pkt);

int modbus_udp_recv(modbus_udp_handle_t *handle, modbus_frame_t *pkt);

#endif /* _MODBUS_UDP_H_ */
