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
#include "modbus-tcp.h"

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
modbus_tcp_close(modbus_tcp_handle_t *handle)
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
modbus_tcp_init(char *host, int port, modbus_tcp_handle_t *handle)
{
    struct timeval timeout;

    if (handle == NULL)
        return -1;
   
    if ((handle->sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) 
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


    // connect
    if (connect(handle->sock, (struct sockaddr *)&(handle->saddr), sizeof(handle->saddr)) == -1)
    {
        close(handle->sock);
        snprintf(modbus_error_str, sizeof(modbus_error_str), "%s: couldn't connect: %s",
                 __PRETTY_FUNCTION__, strerror(errno));
        return -1;
    }


    return 0;
}


//------------------------------------------------------------------------------
//
//
//------------------------------------------------------------------------------
int
modbus_tcp_server_init(int port, modbus_tcp_handle_t *handle)
{
    struct timeval timeout;

    if (handle == NULL)
        return -1;
   
    if ((handle->sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) 
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

    handle->saddr.sin_family      = AF_INET;   
    handle->saddr.sin_addr.s_addr = INADDR_ANY;
    handle->saddr.sin_port        = htons(port);

    // bind the socket
   // if (bind(handle->sock, p->ai_addr, p->ai_addrlen) == -1)
   // {
   //     close(handle->sock);
   //     snprintf(modbus_error_str, sizeof(modbus_error_str), "%s: couldn't connect: %s",
   //              __PRETTY_FUNCTION__, strerror(errno));
   //     return -1;
   // }

/*
   if ( bind( sock, (struct sockaddr *) &sockaddr, sizeof sockaddr ) < 0 ) 
   {
      char s[BUFSIZ];
      sprintf( s, "%s: can't bind socket (%d)", argv[0], sock );
      perror(s);
      exit(__LINE__);
   }
*/

    return 0;
}

int
modbus_tcp_server_listen(modbus_tcp_handle_t *handle)
{
    if (!handle)
        return -1;

    return 0;
}


//------------------------------------------------------------------------------
//
//
//------------------------------------------------------------------------------
int
modbus_tcp_send(modbus_tcp_handle_t *handle, modbus_frame_t *pkt)
{
    char buff[256];
    int len;

    if (pkt == NULL)
        return -1;

	len = modbus_tcp_frame_pack(pkt, buff, sizeof(buff));	

    if (write(handle->sock, buff, len) != len)
    {
	    snprintf(modbus_error_str, sizeof(modbus_error_str),
		         "%s: failed to send modbus tcp frame [%s]", __PRETTY_FUNCTION__, strerror(errno));
        return -1;
    }
       
    return 0;
}

//------------------------------------------------------------------------------
//
//
//------------------------------------------------------------------------------
int
modbus_tcp_recv(modbus_tcp_handle_t *handle, modbus_frame_t *pkt)
{
    char buff[256];
    int len;
       
    //
    // first read the MODBUS header
    //
    if (read(handle->sock, buff, MODBUS_TCP_HEADER_LENGTH) != MODBUS_TCP_HEADER_LENGTH)
    {
	    snprintf(modbus_error_str, sizeof(modbus_error_str),
		         "%s: failed to read modbus header from tcp socket", __PRETTY_FUNCTION__);
        return -1;
    }

    modbus_tcp_header_parse(pkt, buff, MODBUS_TCP_HEADER_LENGTH);

    //
    // read the remaining data, if expected
    //
    len = modbus_frame_get_length(pkt) - 2;

    if (read(handle->sock, &buff[MODBUS_TCP_HEADER_LENGTH], len) != len)
    {
	    snprintf(modbus_error_str, sizeof(modbus_error_str),
		         "%s: failed to read remaining modbus data from tcp socket", __PRETTY_FUNCTION__);
        return -1;
    }

    return modbus_tcp_frame_parse(pkt, buff, MODBUS_TCP_HEADER_LENGTH + len);
}

