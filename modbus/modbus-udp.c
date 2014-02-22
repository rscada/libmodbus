//------------------------------------------------------------------------------
// Copyright (C) 2010, Raditex AB
// All rights reserved.
//
// rSCADA 
// http://www.rSCADA.se
// info@rscada.se
//
//------------------------------------------------------------------------------

#include "modbus.h"
#include "modbus-udp.h"

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


//------------------------------------------------------------------------------
//
//
//------------------------------------------------------------------------------
int
modbus_udp_close(modbus_udp_handle_t *handle)
{
    if (handle == NULL)
        return -1;

    close(handle->sock);

    return 0;
}

//------------------------------------------------------------------------------
//
//
//------------------------------------------------------------------------------
int
modbus_udp_init(char *host, int port, modbus_udp_handle_t *handle)
{
    struct timeval timeout;

    if (handle == NULL)
        return -1;
   
    if ((handle->sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0) 
    {
	    snprintf(modbus_error_str, sizeof(modbus_error_str), "%s: couldn't get socket: %s", 
                 __PRETTY_FUNCTION__, strerror(errno));
      	return -1; 
    }
    
    timeout.tv_sec  = 1;
    timeout.tv_usec = 0;

    if (setsockopt(handle->sock, SOL_SOCKET, SO_RCVTIMEO, (const void *)&timeout, sizeof(struct timeval)) == -1)
    {
        snprintf(modbus_error_str, sizeof(modbus_error_str), "%s: couldn't set receive timeout: %s.",
                 __PRETTY_FUNCTION__, strerror(errno));
        return -1;
    }

/*
    if ((flags = fcntl(handle->sock, F_GETFL, 0)) == -1)
    {
        snprintf(modbus_error_str, sizeof(modbus_error_str), 
                 "%s: couldn't get fd option non-blocking: F_GETFL.", __PRETTY_FUNCTION__);
        return NULL;
    }

    if (fcntl(io->sock, F_SETFL, flags|O_NONBLOCK) == -1) 
    {
        snprintf(modbus_error_str, sizeof(modbus_error_str), 
                 "%s: couldn't set option non-blocking: F_SETFL.", __PRETTY_FUNCTION__);
        return NULL;
    }
 */   

    handle->saddr.sin_family = AF_INET;
    
    if ((handle->addr = gethostbyname(host)) == NULL) 
    {
        snprintf(modbus_error_str, sizeof(modbus_error_str), "%s: couldn't get host: %s: %s",
                 __PRETTY_FUNCTION__, strerror(errno), host);
      	return -1; 
    }
    
    bcopy((char *) handle->addr->h_addr, 
	      (char *)&handle->saddr.sin_addr,
	               handle->addr->h_length);

    handle->saddr.sin_port = htons(port);

    return 0;
}


//------------------------------------------------------------------------------
//
//
//------------------------------------------------------------------------------
int
modbus_udp_send(modbus_udp_handle_t *handle, modbus_frame_t *pkt)
{
    char buff[256];
    int len;

    if (pkt == NULL)
        return -1;

	len = modbus_tcp_frame_pack(pkt, buff, sizeof(buff));	

    if (sendto(handle->sock, buff, len, 0, (struct sockaddr *)&handle->saddr, sizeof(handle->saddr)) != len)
    {
	    snprintf(modbus_error_str, sizeof(modbus_error_str),
		         "%s: failed to send modbus UDP frame", __PRETTY_FUNCTION__);
        return -1;
    }
       
    return 0;
}

//------------------------------------------------------------------------------
//
//
//------------------------------------------------------------------------------
int
modbus_udp_recv(modbus_udp_handle_t *handle, modbus_frame_t *pkt)
{
    socklen_t fromlen;
    struct sockaddr_in caller;
    char buff[256];
    int len;
    
    // read UDP data
    fromlen = sizeof (caller);
    if ((len = recvfrom(handle->sock, buff, sizeof(buff), 0, (struct sockaddr *)&caller, &fromlen)) > 0)
    {    
        return modbus_tcp_frame_parse(pkt, buff, len);
    }

   return -1;
}

