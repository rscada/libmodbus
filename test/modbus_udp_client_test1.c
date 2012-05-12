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
#include "modbus-udp.h"

//#define MODBUS_PACKET_LEN   23	
//#define PACKET_BUFF_SIZE 128

//------------------------------------------------------------------------------
// Execution starts here:
//
//------------------------------------------------------------------------------
int
main(int argc, char **argv)
{
    static char *host;

    char buff[128];
    int port;
    int offset = 0;
    
    modbus_packet_t     *pkt;    
    modbus_udp_handle_t handle;
 
    if (argc != 3)
    {
        printf("%s: usage: %s host port", __PRETTY_FUNCTION__, argv[0]);
        return 0;
    }
 
    host = argv[1];   
    port = atoi(argv[2]);
    
    //
    // setup request packet
    //
    pkt = modbus_packet_new();  
  
    modbus_packet_set_transaction_id(pkt, 0x0BA1);
    modbus_packet_set_function(pkt, MB_FUNC_READ_COIL_STATUS);
    modbus_packet_set_unit(pkt, 0x00);
    buff[offset++] = 0x00; // addr hi
    buff[offset++] = 0x00; // addr lo
    buff[offset++] = 0x00; // # regs hi
    buff[offset++] = 0x04; // # regs lo
    modbus_packet_set_data(pkt, buff, offset);

    modbus_packet_print(pkt);


    if (modbus_udp_init(host, port, &handle) != 0)
    {
        printf("%s: modbus_udp_init failed.\n", __PRETTY_FUNCTION__);
        return 0;
    }

    // Send command
    if (modbus_udp_send(&handle, pkt) != 0)
    {
        printf("%s: modbus_udp_send failed.\n", __PRETTY_FUNCTION__);
        return 0;       
    }

    // recv response
    if (modbus_udp_recv(&handle, pkt) != 0)
    {
        printf("%s: modbus_udp_recv failed.\n", __PRETTY_FUNCTION__);
        return 0;       
    }
   
    modbus_packet_print(pkt);

    printf("FUNCTION = %.2X\n", modbus_packet_get_function(pkt)  & 0xff);
    printf("LENGTH   = %.2X\n", modbus_packet_get_data_size(pkt) & 0xff);
    printf("UNIT     = %.2X\n", modbus_packet_get_unit(pkt)      & 0xff);    

    modbus_packet_free(pkt);

    return 0;
}



