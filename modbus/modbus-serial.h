//------------------------------------------------------------------------------
// Copyright (C) 2014, Robert Johansson <rob@raditex.nu> Raditex Control AB
// All rights reserved.
//
// rSCADA 
// http://www.rSCADA.se
// info@rscada.se
//
//------------------------------------------------------------------------------

#ifndef _MODBUS_SERIAL_H_
#define _MODBUS_SERIAL_H_

//#include <stdlib.h>
//#include <stdio.h>
#include <termios.h>

#include "modbus.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct __modbus_serial_handle {
    
    char *device;
    int fd;
    struct termios t;
    
} modbus_serial_handle_t;

modbus_serial_handle_t *modbus_serial_connect(char *device, long baudrate);
int modbus_serial_disconnect(modbus_serial_handle_t *handle);
int modbus_serial_send(modbus_serial_handle_t *handle, modbus_packet_t *pkt);
int modbus_serial_recv(modbus_serial_handle_t *handle, modbus_packet_t *pkt);
int modbus_serial_set_baudrate(modbus_serial_handle_t *handle, long baudrate);

#ifdef __cplusplus
}
#endif

#endif /* _MODBUS_SERIAL_H_ */
