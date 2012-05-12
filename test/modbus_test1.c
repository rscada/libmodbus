//------------------------------------------------------------------------------
// Copyright (C) 2010, Raditex AB
// All rights reserved.
//
// rSCADA 
// http://www.rSCADA.se
// info@rscada.se
//
//------------------------------------------------------------------------------

//
// libmodbus test1:
// 

#include <stdio.h>

#include "modbus.h"


int
main(int argc, char **argv)
{
    char buff1[128], buff2[128];
    int idx, len, i;
    
    modbus_packet_t *pkt;
    
    //
    //
    //
    idx = 0;
    buff1[idx++] = 0x00; // transaction
    buff1[idx++] = 0x00;
    buff1[idx++] = 0x00; // protocol
    buff1[idx++] = 0x00;      
    buff1[idx++] = 0x00; // length
    buff1[idx++] = 0x06;
    buff1[idx++] = 0xFF; // unit
    buff1[idx++] = 0x01; // func

    buff1[idx++] = 0x00; // address
    buff1[idx++] = 0x01; 
    buff1[idx++] = 0x00; // data
    buff1[idx++] = 0x02; 
   

    //
    // vesam request: 00a1 0000 0006 00 04 007b 0001
    //
    idx = 0;
    buff1[idx++] = 0x00; // transaction
    buff1[idx++] = 0xA1;
    buff1[idx++] = 0x00; // protocol
    buff1[idx++] = 0x00;      
    buff1[idx++] = 0x00; // length
    buff1[idx++] = 0x06;
    buff1[idx++] = 0x00; // unit
    buff1[idx++] = 0x04; // func

    buff1[idx++] = 0x00; // address
    buff1[idx++] = 0x7b; 
    buff1[idx++] = 0x00; // data
    buff1[idx++] = 0x01; 

    pkt = modbus_packet_new();    

    modbus_header_parse(pkt, buff1, idx);
   
    modbus_packet_print(pkt);

    modbus_packet_parse(pkt, buff1, idx);
   
    modbus_packet_print(pkt);

    printf("FUNCTION = %.2X\n", modbus_packet_get_function(pkt));
    printf("LENGTH   = %.2X\n", modbus_packet_get_data_size(pkt));
    printf("UNIT     = %.2X\n", modbus_packet_get_unit(pkt));

    len = modbus_packet_pack(pkt, buff2, sizeof(buff2));
    
    printf("binary data 1 [%d bytes]\n", idx);
    for (i = 0; i < idx; i++)
    {
        printf("%.2X:", buff1[i] & 0xFF);
    }
    printf("\n");
       
    printf("binary data 2 [%d bytes]\n", len);
    for (i = 0; i < idx; i++)
    {
        printf("%.2X:", buff2[i] & 0xFF);
    }
    printf("\n");
    
    modbus_packet_free(pkt);
    
    return 0;
}
