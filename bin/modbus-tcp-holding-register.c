//------------------------------------------------------------------------------
// Copyright (C) 2012, Robert Johansson <rob@raditex.nu>, Raditex Control AB
// All rights reserved.
//
// rSCADA 
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
    printf("\tCOMMAND = read | write\n");    
    printf("\tADDRESS = 0x0000 - 0xFFFF (register address)\n");
    printf("\tVALUE   = 0x0000 - 0xFFFF\n");    
    exit(0);
}

//------------------------------------------------------------------------------
// Read/write a holding register form a MODBUS TCP device
//------------------------------------------------------------------------------
int
main(int argc, char **argv)
{
    static char *host, *command;
    int port = 502;
    uint16_t addr, value, range;
    
    modbus_packet_t     *pkt;    
    modbus_tcp_handle_t handle;
 
    if (argc != 4 && argc != 5)
    {
        printf("Error: Invalid arguments.\n\n");    
        usage(argv[0]);
    }
 
    host    = argv[1];
    command = argv[2];
    
    // setup request packet
    pkt = modbus_packet_new();      
    
    if (strcmp(command, "read") == 0 && argc == 4)
    {
        addr = atoi(argv[3]);
        range = 2;
        modbus_read_holding_registers(pkt, addr, range);          
    }
    else if (strcmp(command, "write") == 0 && argc == 5)
    {
        addr  = atoi(argv[3]);
        value = atoi(argv[4]);        
        modbus_preset_single_register(pkt, addr, value);  
    }    
    else
    {
        printf("Error: Invalid COMMAND: %s\n\n", command);
        usage(argv[0]);
    }

    // setup tcp
    if (modbus_tcp_init(host, port, &handle) != 0)
    {
        printf("%s: modbus_tcp_init failed: %s.\n", __PRETTY_FUNCTION__, modbus_error_str);
        return 0;
    }

    if (debug)
        modbus_packet_print(pkt);
        
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

    if (modbus_packet_verify(pkt) == -1)
    {
        printf("%s: modbus_packet_verify failed: %s\n", __PRETTY_FUNCTION__, modbus_error_str);
        return 0;               
    }

    if (debug)
        modbus_packet_print(pkt);

    if (strcmp(command, "read") == 0)
    {
        value = (pkt->data_buff[3] * 255 + pkt->data_buff[4]);
        printf("0x%.4x\n", value);
    }

    modbus_packet_free(pkt);

    return 0;
}



