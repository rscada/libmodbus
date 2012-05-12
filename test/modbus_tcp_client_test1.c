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
    static char *host;

    int port;
    int i, start_addr, num_inputs;
    
    modbus_packet_t     *pkt;    
    modbus_tcp_handle_t handle;
 
    if (argc != 3)
    {
        printf("%s: usage: %s host port", __PRETTY_FUNCTION__, argv[0]);
        return 0;
    }
 
    host = argv[1];   
    port = atoi(argv[2]);
    
    // setup request packet
    pkt = modbus_packet_new();  
  
    start_addr = 4;    // start address
    num_inputs = 0x0004;    // read four discrete inputs

    //modbus_read_input_status(pkt, start_addr, num_inputs);  
    modbus_read_holding_registers(pkt, start_addr, 0x0001);
    modbus_packet_print(pkt);

    // setup tcp
    if (modbus_tcp_init(host, port, &handle) != 0)
    {
        printf("%s: modbus_udp_init failed.\n", __PRETTY_FUNCTION__);
        return 0;
    }

    // Send command
    if (modbus_tcp_send(&handle, pkt) != 0)
    {
        printf("%s: modbus_udp_send failed.\n", __PRETTY_FUNCTION__);
        return 0;       
    }

    // recv response
    if (modbus_tcp_recv(&handle, pkt) != 0)
    {
        printf("%s: modbus_udp_recv failed.\n", __PRETTY_FUNCTION__);
        return 0;       
    }
   
    modbus_packet_print(pkt);

    for (i = 0; i < num_inputs; i++)
    {
        printf("input[%d] = %d\n", i, modbus_packet_data_bit_get(pkt, i)); 
    }

    modbus_packet_free(pkt);

    return 0;
}



