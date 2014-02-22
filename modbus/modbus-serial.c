//------------------------------------------------------------------------------
// Copyright (C) 2014, Robert Johansson, Raditex Control AB
// All rights reserved.
//
// rSCADA 
// http://www.rSCADA.se
// info@rscada.se
//
//------------------------------------------------------------------------------

#include "modbus.h"
#include "modbus-serial.h"

#include <stdlib.h>
#include <stdio.h>

#include <unistd.h>
#include <fcntl.h>

#include <sys/types.h>

#include <stdio.h>
#include <strings.h>

#include <termios.h>
#include <errno.h>
#include <string.h>

#define PACKET_BUFF_SIZE 2048

//#define MODBUS_HEADER_LENGTH + len
static int debug = 1;

//------------------------------------------------------------------------------
/// Set up a serial connection handle.
//------------------------------------------------------------------------------
modbus_serial_handle_t *
modbus_serial_connect(char *device, long baudrate)
{
    modbus_serial_handle_t *handle;

    if (device == NULL)
    {
        return NULL;
    }

    if ((handle = (modbus_serial_handle_t *)malloc(sizeof(modbus_serial_handle_t))) == NULL)
    {
        fprintf(stderr, "%s: failed to allocate memory for handle\n", __PRETTY_FUNCTION__);
        return NULL;
    }

    handle->device = strdup(device);
    
    //
    // create the SERIAL connection
    //

    // Use blocking read and handle it by serial port VMIN/VTIME setting
    if ((handle->fd = open(handle->device, O_RDWR | O_NOCTTY)) < 0)
    {
        fprintf(stderr, "%s: failed to open tty.", __PRETTY_FUNCTION__);
        return NULL;
    }

    bzero(&(handle->t), sizeof(handle->t));
    handle->t.c_cflag |= (CS8|CREAD|CLOCAL);
    handle->t.c_cflag |= PARENB;
    handle->t.c_cc[VMIN]  = 0;
    handle->t.c_cc[VTIME] = 10;

    if (baudrate == 0)
    {
        cfsetispeed(&(handle->t), B9600);
        cfsetospeed(&(handle->t), B9600);
    }
    else
    {
        modbus_serial_set_baudrate(handle, baudrate);   
    }
   
    tcsetattr(handle->fd, TCSANOW, &(handle->t));

    return handle;    
}

//------------------------------------------------------------------------------
// 
//------------------------------------------------------------------------------
int
modbus_serial_set_baudrate(modbus_serial_handle_t *handle, long baudrate)
{
    if (handle == NULL)
        return -1;

    switch (baudrate)
    {
        case 300:
            cfsetispeed(&(handle->t), B300);
            cfsetospeed(&(handle->t), B300);
            return 0;

        case 1200:
            cfsetispeed(&(handle->t), B1200);
            cfsetospeed(&(handle->t), B1200);
            return 0;

        case 2400:
            cfsetispeed(&(handle->t), B2400);
            cfsetospeed(&(handle->t), B2400);
            return 0;

        case 9600:
            cfsetispeed(&(handle->t), B9600);
            cfsetospeed(&(handle->t), B9600);
            return 0;
            
        case 38400:
            cfsetispeed(&(handle->t), B38400);
            cfsetospeed(&(handle->t), B38400);
            return 0;

        case 57600:
            cfsetispeed(&(handle->t), B57600);
            cfsetospeed(&(handle->t), B57600);
            return 0;
                        
       default:
            return -1; // unsupported baudrate
    }
}


//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
int
modbus_serial_disconnect(modbus_serial_handle_t *handle)
{
    if (handle == NULL)
    {
        return -1;
    }

    close(handle->fd);

    if (handle->device)
        free(handle->device);    
        
    free(handle);

    return 0;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
int
modbus_serial_send(modbus_serial_handle_t *handle, modbus_packet_t *pkt)
{
    u_char buff[PACKET_BUFF_SIZE];
    int len, ret;

    if (handle == 0 || pkt == NULL)
        return -1;

    if ((len = modbus_packet_pack(pkt, buff, sizeof(buff))) == -1)
    {
        fprintf(stderr, "%s: modbus_frame_t_pack failed\n", __PRETTY_FUNCTION__);
        return -1;
    }
    
    if ((ret = write(handle->fd, buff, len)) != len)
    {   
	    snprintf(modbus_error_str, sizeof(modbus_error_str),
	             "%s: Failed to write frame to serial device (ret = %d: %s)",
	             __PRETTY_FUNCTION__, ret, strerror(errno));
        return -1;
    }

    if (debug)
    {
        int i;
        printf("%s: Wrote %d bytes: ", __PRETTY_FUNCTION__, len);
        for (i = 0; i < len; i++)
        {
            printf("0x%.2x ", buff[i] & 0xFF);
        }
        printf("\n");    
    }

    tcdrain(handle->fd);

    return 0;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
int
modbus_serial_recv(modbus_serial_handle_t *handle, modbus_packet_t *pkt)
{
    u_char buff[PACKET_BUFF_SIZE];
    int len, ret, i;
    
    bzero((void *)buff, sizeof(buff));
        
    //
    // first read the MODBUS header
    //
    if (read(handle->fd, buff, MODBUS_HEADER_LENGTH) != MODBUS_HEADER_LENGTH)
    {
	    snprintf(modbus_error_str, sizeof(modbus_error_str),
		         "%s: failed to read modbus header from tcp socket", __PRETTY_FUNCTION__);
        return -1;
    }

    modbus_header_parse(pkt, buff, MODBUS_HEADER_LENGTH);

    //
    // read the remaining data, if expected
    //
    len = modbus_packet_get_length(pkt) - 2;

    if (read(handle->fd, &buff[MODBUS_HEADER_LENGTH], len) != len)
    {
	    snprintf(modbus_error_str, sizeof(modbus_error_str),
		         "%s: failed to read remaining modbus data from tcp socket", __PRETTY_FUNCTION__);
        return -1;
    }

    if (debug)
    {
        printf("%s: Read %d bytes: ", __PRETTY_FUNCTION__, MODBUS_HEADER_LENGTH + len);
        for (i = 0; i < MODBUS_HEADER_LENGTH + len; i++)
        {
            printf("0x%.2x ", buff[i] & 0xFF);            
        }
        printf("\n");
    }    

    return modbus_packet_parse(pkt, buff, MODBUS_HEADER_LENGTH + len);
}


