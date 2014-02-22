//------------------------------------------------------------------------------
// Copyright (C) 2012, Robert Johansson <rob@raditex.nu>, Raditex Control AB
// All rights reserved.
//
// This file is part of rSCADA.
// http://www.rSCADA.se
// info@rscada.se
//
//------------------------------------------------------------------------------

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <syslog.h>
#include <string.h>

#include <modbus/modbus.h>
#include <modbus/modbus-tcp.h>

static int debug = 0;

void
usage(char *program_name)
{
    printf("usage: %s HOST COMMAND ADDRESS [VALUE]\n", program_name);
    printf("\tHOST    = ip address or FQDN\n");
    printf("\tCOMMAND = read | write | readrange\n");    
    printf("\tADDRESS = 0 - X (coil address)\n");
    printf("\tVALUE   = 0 | 1 (new cold value)\n");
    exit(0);
}

//------------------------------------------------------------------------------
// Read/Write the state of discrete outputs ("coil" in MODBUS language...)
//------------------------------------------------------------------------------
int
main(int argc, char **argv)
{
    static char *host, *command;
    int port = 502, i;
    uint16_t addr, value, range = 1;
    
    modbus_frame_t     *pkt;    
    modbus_tcp_handle_t handle;
 
    if (argc != 4 && argc != 5)
    {
        printf("Error: Invalid arguments.\n\n");    
        usage(argv[0]);
    }
 
    host    = argv[1];
    command = argv[2];
    
    if (strcmp(command, "write") == 0)
    {
        if (argc != 5)
        {
           printf("Error: Invalid arguments: COMMAND write requies additional argument VALUE\n\n");    
           usage(argv[0]);        
        }
        
        addr  = atoi(argv[3]);
        value = atoi(argv[4]);    
    }
    else if (strcmp(command, "read") == 0)
    {
        addr = atoi(argv[3]);    
    }
    else if (strcmp(command, "readrange") == 0)
    {
        addr = atoi(argv[3]);
        range = 8;
    }    
    else
    {
        printf("Error: Invalid COMMAND: %s\n\n", command);
        usage(argv[0]);
    }
        
    // setup request frame
    pkt = modbus_frame_new();  
  
    if (strcmp(command, "write") == 0)
    {
        modbus_force_single_coil(pkt, addr, value);
    }
    else
    {
        modbus_read_coil_status(pkt, addr, range);  
    }

    // setup tcp
    if (modbus_tcp_init(host, port, &handle) != 0)
    {
        printf("%s: modbus_tcp_init failed: %s.\n", __PRETTY_FUNCTION__, modbus_error_str);
        return 0;
    }

    if (debug)
        modbus_frame_print(pkt);
        
    // Send command
    if (modbus_tcp_send(&handle, pkt) != 0)
    {
        printf("%s: modbus_tcp_send failed: %s.\n", __PRETTY_FUNCTION__, modbus_error_str);
        return 0;       
    }

    // recv response
    if (modbus_tcp_recv(&handle, pkt) != 0)
    {
        printf("%s: modbus_tcp_recv failed: %s.\n", __PRETTY_FUNCTION__, modbus_error_str);
        return 0;       
    }

    if (modbus_frame_verify(pkt) == -1)
    {
        printf("%s: modbus_frame_verify failed: %s\n", __PRETTY_FUNCTION__, modbus_error_str);
        return 0;               
    }

    if (debug)
        modbus_frame_print(pkt);

    if (strcmp(command, "write") == 0)
    {
        ; //    
    }
    else // a read command
    {
        for (i = range - 1; i >= 0; i--)
        {
            printf("%d ", pkt->data_buff[1] & (0x01<<i) ? 1 : 0);
        }
        printf("\n");
    }
    modbus_frame_free(pkt);

    return 0;
}



