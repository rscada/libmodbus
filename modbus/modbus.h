//------------------------------------------------------------------------------
// Copyright (C) 2010-2012, Robert Johansson <rob@raditex.nu> Raditex Control AB
// All rights reserved.
//
// rSCADA 
// http://www.rSCADA.se
// info@rscada.se
//
//------------------------------------------------------------------------------

#ifndef _MODBUS_H_
#define _MODBUS_H_

#include <stdlib.h>
#include <sys/types.h>
#include <stdint.h>

extern char modbus_error_str[256];

//
// Modbus TCP Frame Format
// Name 	                Length 	    Function
// Transaction Identifier 	2 bytes 	For synchronization between messages of server & client
// Protocol Identifier 	    2 bytes 	Zero for MODBUS/TCP
// Length Field 	        2 bytes 	Number of remaining bytes in this frame
// Unit Identifier 	        1 byte 	    Slave Address (255 if not used)
// Function code 	        1 byte 	    Function codes as in other variants
// Data bytes 	            n bytes 	Data as response or commands
//
// Source: http://en.wikipedia.org/wiki/Modbus
//

#define MODBUS_HEADER_LENGTH 8

typedef struct _modbus_header {

    char transaction1;
    char transaction2;

    char protocol1;
    char protocol2;

    char length1;
    char length2;

    char unit;
    
    char func_code;
    
} modbus_header_t;

typedef struct _modbus_packet {

    modbus_header_t hdr;

    uint16_t   data_buff_len;
    char *data_buff;    
    
} modbus_packet_t;

//
// Modbus Function codes:
//
#define MB_FUNC_READ_COIL_STATUS        0x01
#define MB_FUNC_READ_INPUT_STATUS       0x02
#define MB_FUNC_READ_HOLDING_REGISTER   0x03
#define MB_FUNC_READ_INPUT_REGISTERS    0x04
#define MB_FUNC_FORCE_SINGLE_COIL       0x05
#define MB_FUNC_PRESET_SINGLE_REGISTER  0x06
#define MB_FUNC_DIAGNOSTICS             0x08

#define MB_FUNC_READ_EXCEPTION_STATUS       0x07
#define MB_FUNC_FORCE_MULTIPLE_COILS        0x0F
#define MB_FUNC_PRESET_MULTIPLE_REGISTERS   0x10
#define MB_FUNC_REPORT_SLAVE_ID             0x11

#define MB_FUNC_FETCH_COM_EVENT_CTRL        0x0B
#define MB_FUNC_FETCH_COM_EVENT_LOG         0x0C
#define MB_FUNC_READ_GENERAL_REF            0x14
#define MB_FUNC_WRITE_GENERAL_REF           0x15
#define MB_FUNC_MASK_WRITE_4XREG            0x16
#define MB_FUNC_RW_4XREGS                   0x17
#define MB_FUNC_READ_FIFO_QUEUE             0x18

//
// Modbus protocol exceptions (error codes)
//
#define MB_ERROR_ILLEGAL_FUNCTION        -0x01
#define MB_ERROR_ILLEGAL_DATA_ADDRESS    -0x02
#define MB_ERROR_ILLEGAL_DATA_VALUE      -0x03
#define MB_ERROR_SLAVE_DEVICE_FAILURE    -0x04
#define MB_ERROR_SERVER_FAILURE          -0x04
#define MB_ERROR_ACKNOWLEDGE             -0x05
#define MB_ERROR_SLAVE_DEVICE_BUSY       -0x06
#define MB_ERROR_SERVER_BUSY             -0x06
#define MB_ERROR_NEGATIVE_ACKNOWLEDGE    -0x07
#define MB_ERROR_MEMORY_PARITY_ERROR     -0x08
#define MB_ERROR_GATEWAY_PROBLEM_PATH    -0x0A
#define MB_ERROR_GATEWAY_PROBLEM_TARGET  -0x0B

#define MB_ERROR_FLAG                     0x80

//
// Functions for managing the modbus packets
//
modbus_packet_t *modbus_packet_new();

void modbus_packet_free(modbus_packet_t *pkt);
void modbus_packet_print(modbus_packet_t *pkt);

uint16_t  modbus_header_parse(modbus_packet_t *pkt, char *data, size_t data_size);
uint16_t  modbus_packet_parse(modbus_packet_t *pkt, char *data, size_t data_size);
uint16_t  modbus_packet_pack (modbus_packet_t *pkt, char *data_buff, size_t data_buff_size);
uint16_t  modbus_packet_verify(modbus_packet_t *pkt);


//
// Functions for manipulating the modbus packets
//
uint16_t  modbus_packet_get_data_size     (modbus_packet_t *pkt);
uint16_t  modbus_packet_get_transaction_id(modbus_packet_t *pkt);
uint8_t   modbus_packet_get_unit          (modbus_packet_t *pkt);
uint8_t   modbus_packet_get_function      (modbus_packet_t *pkt);
uint16_t  modbus_packet_get_length        (modbus_packet_t *pkt);

uint16_t  modbus_packet_set_data     (modbus_packet_t *pkt, char *data, size_t data_size);
uint16_t  modbus_packet_set_unit     (modbus_packet_t *pkt, char val);
uint16_t  modbus_packet_set_transaction_id(modbus_packet_t *pkt, uint16_t trans_id);
uint16_t  modbus_packet_set_function (modbus_packet_t *pkt, char val);
uint16_t  modbus_packet_set_length   (modbus_packet_t *pkt, uint16_t len);

//
// Create modbus packages
//
uint16_t modbus_read_holding_registers   (modbus_packet_t *pkt, uint16_t base_addr, uint16_t num_regs);
uint16_t modbus_preset_single_register   (modbus_packet_t *pkt, uint16_t base_addr, uint16_t num_regs);
uint16_t modbus_read_input_status        (modbus_packet_t *pkt, uint16_t base_addr, uint16_t num_regs);
uint16_t modbus_read_coil_status         (modbus_packet_t *pkt, uint16_t base_addr, uint16_t num_regs);
uint16_t modbus_force_single_coil        (modbus_packet_t *pkt, uint16_t base_addr, uint16_t value);
uint16_t modbus_diagnostics              (modbus_packet_t *pkt, uint16_t subfunc,   char *data, uint16_t data_len);

uint16_t modbus_response_read_holding_registers(modbus_packet_t *pkt, uint16_t *value_tbl, uint16_t tbl_size);

uint16_t modbus_exception(modbus_packet_t *pkt, uint16_t error_code, uint16_t exception_code);

//
// parsing functions
//
uint16_t modbus_read_holding_register_get_address(modbus_packet_t *pkt);
uint16_t modbus_read_holding_register_get_range  (modbus_packet_t *pkt);
uint16_t modbus_error_get_exception_code(modbus_packet_t *pkt);

uint16_t modbus_packet_data_decode16(modbus_packet_t *pkt, uint16_t offset);

uint16_t modbus_packet_data_num_bytes(modbus_packet_t *pkt);
uint16_t modbus_packet_reg8_get (modbus_packet_t *pkt, uint16_t n);
uint16_t modbus_packet_reg16_get(modbus_packet_t *pkt, uint16_t n);
uint16_t modbus_packet_reg32_get(modbus_packet_t *pkt, uint16_t n);

uint16_t modbus_packet_data_bit_get(modbus_packet_t *pkt, uint16_t n);

//
// Read input/holding register functions: obsolete....
//
uint16_t modbus_packet_read_register_get_address(modbus_packet_t *pkt);
uint16_t modbus_packet_read_register_set_address(modbus_packet_t *pkt, uint16_t address);
uint16_t modbus_packet_read_register_get_data(modbus_packet_t *pkt, uint16_t offset);
uint16_t modbus_packet_read_register_set_data(modbus_packet_t *pkt, char *data, uint16_t data_len);


#endif /* _MODBUS_H_ */

