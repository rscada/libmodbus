//------------------------------------------------------------------------------
// Copyright (c) 2009 Robert Johansson <rob@raditex.se>, Raditex AB.
// All rights reserved.
//
// $Id$
//------------------------------------------------------------------------------

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <syslog.h>
#include <string.h>

#include "modbus.h"
#include "modbus-tcp.h"

//------------------------------------------------------------------------------
// Execution starts here:
//
//------------------------------------------------------------------------------
int
main(int argc, char **argv)
{
    //static char *host;
    int port;
    
    modbus_packet_t     *pkt;    
    modbus_tcp_handle_t handle;
 
    if (argc != 2)
    {
        printf("%s: usage: %s port", __PRETTY_FUNCTION__, argv[0]);
        return 0;
    }
 
 //   host = argv[1];   
    port = atoi(argv[1]);
    
    // setup packet
    pkt = modbus_packet_new();  

    // setup server socket
    if (modbus_tcp_server_init(port, &handle) != 0)
    {
        printf("%s: modbus_tcp_server_init failed.\n", __PRETTY_FUNCTION__);
        return -1;
    }

    while (1)
    {
        int client_sock;

        // listen for and accept connections
        if ((client_sock = modbus_tcp_server_listen(&handle)) == -1)
        {
            printf("%s: modbus_tcp_server_listen failed.\n", __PRETTY_FUNCTION__);
            return -1;
        }

        // receive
        if (modbus_tcp_recv(&handle, pkt) != 0)
        {
            printf("%s: modbus_tcp_recv failed.\n", __PRETTY_FUNCTION__);
            return -1;       
        }

        modbus_packet_print(pkt);

        // echo request
        if (modbus_tcp_send(&handle, pkt) != 0)
        {
            printf("%s: modbus_tcp_send failed.\n", __PRETTY_FUNCTION__);
            return -1;
        }

    }

    modbus_packet_free(pkt);

    return 0;
}



