//------------------------------------------------------------------------------
// Copyright (C) 2010-2012, Robert Johansson <rob@raditex.nu> Raditex Control AB
// All rights reserved.
//
// rSCADA 
// http://www.rSCADA.se
// info@rscada.se
//
//------------------------------------------------------------------------------

#include <stdio.h>
#include <string.h>
#include <stdint.h>

#include "modbus.h"

char modbus_error_str[256];

//------------------------------------------------------------------------------
// Allocate a new modbus-tcp frame_t data structure
//------------------------------------------------------------------------------
modbus_frame_t *
modbus_frame_new()
{
    modbus_frame_t *pkt;
    
    if ((pkt = malloc(sizeof(modbus_frame_t))) != NULL)
    {
        pkt->hdr_tcp.transaction1 = 0x00;
        pkt->hdr_tcp.transaction2 = 0x00;
        pkt->hdr_tcp.protocol1 = 0x00;
        pkt->hdr_tcp.protocol2 = 0x00;
        pkt->hdr_tcp.length1 = 0x00;
        pkt->hdr_tcp.length2 = 0x00;
        pkt->hdr_tcp.unit = 0xFF;
        pkt->hdr_tcp.func_code = 0x00;
        
        pkt->data_buff     = NULL;
        pkt->data_buff_len = 0;
        return pkt;
    }

    return NULL;
}

//------------------------------------------------------------------------------
// Free allocated data associated with the modbus-tcp frame_t data structure
//------------------------------------------------------------------------------
void
modbus_frame_free(modbus_frame_t *pkt)
{
    if (pkt)
    {
        if (pkt->data_buff)
        {
            free(pkt->data_buff);
        }
        free(pkt);
    }
}

//------------------------------------------------------------------------------
// dump frame in a human-readable form
//------------------------------------------------------------------------------
void
modbus_frame_print(modbus_frame_t *pkt)
{
    uint16_t i;
    
    if (pkt)
    {
        printf("MODBUS: frame DUMP\n");
        printf("MODBUS: transaction1  = %.2X\n", pkt->hdr_tcp.transaction1 & 0xFF);
        printf("MODBUS: transaction2  = %.2X\n", pkt->hdr_tcp.transaction2 & 0xFF);
        printf("MODBUS: protocol1     = %.2X\n", pkt->hdr_tcp.protocol1 & 0xFF);
        printf("MODBUS: protocol2     = %.2X\n", pkt->hdr_tcp.protocol2 & 0xFF);
        printf("MODBUS: length1       = %.2X\n", pkt->hdr_tcp.length1   & 0xFF);
        printf("MODBUS: length2       = %.2X\n", pkt->hdr_tcp.length2   & 0xFF);
        printf("MODBUS: unit id       = %.2X\n", pkt->hdr_tcp.unit      & 0xFF);
        printf("MODBUS: function code = %.2X\n", pkt->hdr_tcp.func_code & 0xFF);
        
        printf("MODBUS: # bytes data  = %.2X\n", pkt->data_buff_len & 0xFF);
        if (pkt->data_buff_len > 0)
        {
            printf("MODBUS: data :");
            for (i = 0; i < pkt->data_buff_len; i++)
            {
                printf("%.2X:", pkt->data_buff[i] & 0xFF);
            }
            printf("\n");
        }
        printf("MODBUS: DONE\n\n");
    }
}

void
modbus_rtu_frame_print(modbus_frame_t *pkt)
{
    uint16_t i;
    
    if (pkt)
    {
        printf("MODBUS: RTU frame DUMP\n");
        printf("MODBUS: unit id       = %.2X\n", pkt->hdr_rtu.unit      & 0xFF);
        printf("MODBUS: function code = %.2X\n", pkt->hdr_rtu.func_code & 0xFF);
        
        printf("MODBUS: # bytes data  = %.2X\n", pkt->data_buff_len & 0xFF);
        if (pkt->data_buff_len > 0)
        {
            printf("MODBUS: data :");
            for (i = 0; i < pkt->data_buff_len; i++)
            {
                printf("%.2X:", pkt->data_buff[i] & 0xFF);
            }
            printf("\n");
        }
        printf("MODBUS: CRC = %.2X %.2X\n", pkt->crc1 & 0xFF, pkt->crc2 & 0xFF);
        printf("MODBUS: DONE\n\n");
    }
}



//------------------------------------------------------------------------------
// Parse binary data and setup frame data structure
//------------------------------------------------------------------------------
uint16_t
modbus_tcp_header_parse(modbus_frame_t *pkt, char *data, size_t data_size)
{   
    if (data_size < MODBUS_TCP_HEADER_LENGTH)
    {
        // too small data. it cannot fit a proper modbus frame
        return -1;
    }
    
    memcpy((void *)&pkt->hdr_tcp, (void *)data, MODBUS_TCP_HEADER_LENGTH);
       
    return 0;
}

//------------------------------------------------------------------------------
// Parse binary data and setup frame_t data structure
//------------------------------------------------------------------------------
uint16_t
modbus_tcp_frame_parse(modbus_frame_t *pkt, char *data, size_t data_size)
{
    uint16_t len, i, idx;
    
    if (data_size < MODBUS_TCP_HEADER_LENGTH)
    {
        // too small data. it cannot fit a proper modbus frame
        printf("data_size(%zu) < MODBUS_TCP_HEADER_LENGTH\n", data_size);
        return -1;
    }
    
    idx = 0;
    
    pkt->hdr_tcp.transaction1 = data[idx++];
    pkt->hdr_tcp.transaction2 = data[idx++];
    pkt->hdr_tcp.protocol1    = data[idx++];
    pkt->hdr_tcp.protocol2    = data[idx++];     
    pkt->hdr_tcp.length1      = data[idx++];
    pkt->hdr_tcp.length2      = data[idx++];
    pkt->hdr_tcp.unit         = data[idx++];
    pkt->hdr_tcp.func_code    = data[idx++];
    
    //if (modbus_header_parse(pkt, data, data_size) == -1)
    //{
    //    return -1;
    //}
    //idx = MODBUS_TCP_HEADER_LENGTH
        
    //len = (pkt->hdr_tcp.length2 + pkt->hdr_tcp.length1 * 256) - 2;
    len = modbus_frame_get_length(pkt) - 2;  


    if ((size_t)(MODBUS_TCP_HEADER_LENGTH + len) != data_size)
    {
        printf("%s: frame length mismatch (data_size = %zu, expected = %zu)\n", __PRETTY_FUNCTION__, data_size, (size_t)(MODBUS_TCP_HEADER_LENGTH + len));
        return -1;
    }
    
    if (pkt->data_buff)
    {
        free(pkt->data_buff);
    }

    pkt->data_buff_len = len;

    if (len > 0)
    {
        if ((pkt->data_buff = malloc(len)) == NULL)
        {
            printf("%s: failed to allocate data_buff (%d)\n", __PRETTY_FUNCTION__, len);
            return -1;
        }
    
        for (i = 0; i < len; i++)
        {
            pkt->data_buff[i] = data[idx++];  
        }
    }
    
    return 0;
}

//------------------------------------------------------------------------------
// Generate binary frame_t from frame_t data structure
//------------------------------------------------------------------------------
uint16_t
modbus_tcp_frame_pack(modbus_frame_t *pkt, char *data_buff, size_t data_buff_size)
{
    uint16_t pkt_len, idx, i;
    
    if (pkt == NULL)
    {
        return -1;
    }
    
    pkt_len = MODBUS_TCP_HEADER_LENGTH + pkt->data_buff_len;
    
    if ((size_t)pkt_len > data_buff_size)
    {
        return -1;
    }

    idx = 0;
    
    data_buff[idx++] = pkt->hdr_tcp.transaction1;
    data_buff[idx++] = pkt->hdr_tcp.transaction2;    
    data_buff[idx++] = pkt->hdr_tcp.protocol1;
    data_buff[idx++] = pkt->hdr_tcp.protocol2;     
    data_buff[idx++] = pkt->hdr_tcp.length1;
    data_buff[idx++] = pkt->hdr_tcp.length2;
    data_buff[idx++] = pkt->hdr_tcp.unit;
    data_buff[idx++] = pkt->hdr_tcp.func_code;
    
    for (i = 0; i < pkt->data_buff_len; i++)
    {
        data_buff[idx++] = pkt->data_buff[i];
    }
    
    return idx;
}

//------------------------------------------------------------------------------
// Parse binary data and setup frame data structure
//------------------------------------------------------------------------------
uint16_t
modbus_rtu_header_parse(modbus_frame_t *pkt, char *data, size_t data_size)
{   
    if (data_size < MODBUS_RTU_HEADER_LENGTH)
    {
        // too small data. it cannot fit a proper modbus frame
        return -1;
    }
    
    memcpy((void *)&pkt->hdr_rtu, (void *)data, MODBUS_RTU_HEADER_LENGTH);
       
    return 0;
}

//------------------------------------------------------------------------------
// Parse binary data and setup frame_t data structure
//------------------------------------------------------------------------------
uint16_t
modbus_rtu_frame_parse(modbus_frame_t *pkt, char *data, size_t data_size)
{
    uint16_t len, i, idx;
    
    if (data_size < MODBUS_RTU_HEADER_LENGTH)
    {
        // too small data. it cannot fit a proper modbus frame
        printf("data_size(%zu) < MODBUS_RTU_HEADER_LENGTH\n", data_size);
        return -1;
    }
    
    idx = 0;
    
    pkt->hdr_tcp.unit         = data[idx++];
    pkt->hdr_tcp.func_code    = data[idx++];
            
    //len = (pkt->hdr_tcp.length2 + pkt->hdr_tcp.length1 * 256) - 2;
    len = data_size - 4;

    //if ((size_t)(MODBUS_TCP_HEADER_LENGTH + len) != data_size)
    //{
    //    printf("%s: frame length mismatch (data_size = %zu, expected = %zu)\n", __PRETTY_FUNCTION__, data_size, (size_t)(MODBUS_TCP_HEADER_LENGTH + len));
    //    return -1;
    //}
    
    if (pkt->data_buff)
    {
        free(pkt->data_buff);
    }

    pkt->data_buff_len = len;

    if (len > 0)
    {
        if ((pkt->data_buff = malloc(len)) == NULL)
        {
            printf("%s: failed to allocate data_buff (%d)\n", __PRETTY_FUNCTION__, len);
            return -1;
        }
    
        for (i = 0; i < len; i++)
        {
            pkt->data_buff[i] = data[idx++];  
        }
    }

    pkt->crc1 = data[idx++];
    pkt->crc2 = data[idx++];

    pkt->frame_type = MD_FRAME_TYPE_RTU;
 
    return 0;
}

//------------------------------------------------------------------------------
// Generate binary frame_t from frame_t data structure
//------------------------------------------------------------------------------
uint16_t
modbus_rtu_frame_pack(modbus_frame_t *pkt, char *data_buff, size_t data_buff_size)
{
    uint16_t idx, i, c;
    
    if (pkt == NULL)
    {
        return -1;
    }

    if (data_buff == NULL)
    {
        return -1;
    }

    idx = 0;
    
    data_buff[idx++] = pkt->hdr_tcp.unit;
    data_buff[idx++] = pkt->hdr_tcp.func_code;
    
    for (i = 0; i < pkt->data_buff_len; i++)
    {
        data_buff[idx++] = pkt->data_buff[i];
    }
    
    c = crc16(data_buff, idx);
    
    data_buff[idx++] = 0xff & c;
    data_buff[idx++] = 0xff & (c>>8);
    
    return idx;
}

//------------------------------------------------------------------------------
// MODBUS RTU CRC16
//------------------------------------------------------------------------------
uint16_t 
crc16(char *data_buff, size_t data_buff_size)
{
    int i, j;
    uint16_t crc = 0xFFFF;
 
    for (i = 0; i < data_buff_size; i++)
    {
        crc ^= (uint16_t)data_buff[i];
 
        for (j = 8; j; j--) 
        {
            if ((crc & 0x0001) != 0) 
            {
                crc >>= 1;
                crc ^= 0xA001;
            }
            else
            {
                crc >>= 1;
            }
        }
    }

    return crc;  
}

//------------------------------------------------------------------------------
// 
//------------------------------------------------------------------------------
uint16_t
modbus_frame_get_data_size(modbus_frame_t *pkt)
{
    if (pkt)
    {
        return pkt->hdr_tcp.length1 * 255 + pkt->hdr_tcp.length2 - 2;
    }

    return -1;
}

//------------------------------------------------------------------------------
// 
//------------------------------------------------------------------------------
uint16_t
modbus_frame_get_transaction_id(modbus_frame_t *pkt)
{
    if (pkt)
    {
        return pkt->hdr_tcp.transaction1 * 255 + pkt->hdr_tcp.transaction2;
    }

    return -1;
}

//------------------------------------------------------------------------------
// 
//------------------------------------------------------------------------------
uint8_t
modbus_frame_get_unit(modbus_frame_t *pkt)
{
    if (pkt)
    {
        return pkt->hdr_tcp.unit;
    }
    return -1;
}

//------------------------------------------------------------------------------
// 
//------------------------------------------------------------------------------
uint8_t
modbus_frame_get_function(modbus_frame_t *pkt)
{
    if (pkt)
    {
        return pkt->hdr_tcp.func_code;
    }
    return -1;
}

//------------------------------------------------------------------------------
// 
//------------------------------------------------------------------------------
uint16_t
modbus_frame_get_length(modbus_frame_t *pkt)
{
    if (pkt)
    {
        return  pkt->hdr_tcp.length1 * 255 + pkt->hdr_tcp.length2;
    }
    
    return -1;
}

//------------------------------------------------------------------------------
// 
//------------------------------------------------------------------------------
uint16_t
modbus_frame_set_data(modbus_frame_t *pkt, char *data, size_t data_size)
{   
    if ((pkt->data_buff = malloc(data_size)) == NULL)
    {
        printf("%s: failed to allocate data_buff (%zu)\n", __PRETTY_FUNCTION__, data_size);
        return -1;
    }  
    
    memcpy((void *)pkt->data_buff, (void *)data, data_size);
    pkt->data_buff_len = data_size;
    
    modbus_frame_set_length(pkt, data_size + 2);
    return 0;    
}

//------------------------------------------------------------------------------
// 
//------------------------------------------------------------------------------
uint16_t
modbus_frame_set_unit(modbus_frame_t *pkt, char val)
{
    if (pkt)
    {
        pkt->hdr_tcp.unit = val;
    }
    
    return 0;
}


//------------------------------------------------------------------------------
// 
//------------------------------------------------------------------------------
uint16_t
modbus_frame_set_function(modbus_frame_t *pkt, char val)
{
    if (pkt)
    {
        pkt->hdr_tcp.func_code = val;
    }
    
    return 0;
}

//------------------------------------------------------------------------------
// 
//------------------------------------------------------------------------------
uint16_t
modbus_frame_set_length(modbus_frame_t *pkt, uint16_t len)
{
    if (pkt)
    {
        pkt->hdr_tcp.length1 = (len>>8) & 0xff;
        pkt->hdr_tcp.length2 =  len     & 0xff;
    }
    
    return 0;
}

//------------------------------------------------------------------------------
// 
//------------------------------------------------------------------------------
uint16_t
modbus_frame_set_transaction_id(modbus_frame_t *pkt, uint16_t id)
{
    if (pkt)
    {
        pkt->hdr_tcp.transaction1 = (id>>8) & 0xff;
        pkt->hdr_tcp.transaction2 =  id     & 0xff;
    }
    
    return 0;
}


//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//
// Parse data functions
//
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

//
// get number of bytes 
//
uint16_t
modbus_frame_data_num_bytes(modbus_frame_t *pkt)
{
    if (pkt == NULL || pkt->data_buff == NULL || pkt->data_buff_len == 0)
        return -1;

    return pkt->data_buff[0];
}

//
// get n'th 8 bit register from the data
//
uint16_t
modbus_frame_reg8_get(modbus_frame_t *pkt, uint16_t n)
{
    if (pkt == NULL || pkt->data_buff == NULL || pkt->data_buff_len < n+2)
        return -1;

    return (uint16_t)pkt->data_buff[n + 1];    
}

//
// get n'th 16 bit register from the data
//
uint16_t
modbus_frame_reg16_get(modbus_frame_t *pkt, uint16_t n)
{
    uint16_t val;

    if (pkt == NULL || pkt->data_buff == NULL || pkt->data_buff_len < 2*(n+1)+1)
        return -1;

    val = (uint16_t)((pkt->data_buff[2*n + 1] & 0xFF) << 8) +
          (uint16_t)(pkt->data_buff[2*n + 2] & 0xFF);

    return val;
}

uint16_t
modbus_frame_data_decode16(modbus_frame_t *pkt, uint16_t offset)
{
    uint16_t val;

    if (pkt == NULL || pkt->data_buff == NULL || pkt->data_buff_len < offset+1)
        return -1;

    val = (uint16_t)((pkt->data_buff[offset]     & 0xFF) << 8) +
          (uint16_t)( pkt->data_buff[offset + 1] & 0xFF);

    return val;
}



//
// get n'th 32 bit register from the data
//
uint16_t
modbus_frame_reg32_get(modbus_frame_t *pkt, uint16_t n)
{
    uint16_t val;

    if (pkt == NULL || pkt->data_buff == NULL || pkt->data_buff_len < 4*(n+1) + 1)
        return -1;

    val = (uint16_t)((pkt->data_buff[4*n + 1] & 0xFF) << 24) + 
          (uint16_t)((pkt->data_buff[4*n + 2] & 0xFF) << 16) +
          (uint16_t)((pkt->data_buff[4*n + 3] & 0xFF) <<  8) + 
          (uint16_t)((pkt->data_buff[4*n + 4] & 0xFF));

    return val;
}

//
// get nth bit from discrete data: first byte is the number byte,
// byte 1 to n are packed with bits. this function extract the nth 
// bit from the n-1 bytes of packed bits.
//
// data[0] = number of bits
// data[1] = DDDDDDDDb (eight first bits)
// data[2] = DDDDDDDDb (bit 9-16)
// ...
//
uint16_t
modbus_frame_data_bit_get(modbus_frame_t *pkt, uint16_t n)
{
    uint16_t byte_addr;

    if (pkt == NULL || pkt->data_buff == NULL || pkt->data_buff_len < n/8 + 1)
        return -1;

    byte_addr = n / 8; 

    return (pkt->data_buff[1 + byte_addr] & (1 << (n%8))) ? 1 : 0;
}


//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//
// Read holding register functions
//
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

uint16_t
modbus_read_holding_registers(modbus_frame_t *pkt, uint16_t base_addr, uint16_t num_regs)
{
    char buff[128];
    uint16_t offset = 0;

    if (pkt == NULL)
        return -1;

    //modbus_frame_set_transaction_id(pkt, 0x1E02); // random?
    modbus_frame_set_function(pkt, MB_FUNC_READ_HOLDING_REGISTERS);
    modbus_frame_set_unit(pkt, 0x00);
    buff[offset++] = (base_addr>>8) & 0xFF; // addr hi
    buff[offset++] = (base_addr)    & 0xFF; // addr lo
    buff[offset++] = (num_regs>>8)  & 0xFF; // # regs hi
    buff[offset++] = (num_regs)     & 0xFF; // # regs lo
    modbus_frame_set_data(pkt, buff, offset);
  
    return 0;   
}


uint16_t
modbus_preset_single_register(modbus_frame_t *pkt, uint16_t base_addr, uint16_t num_regs)
{
    char buff[128];
    uint16_t offset = 0;

    if (pkt == NULL)
        return -1;

    //modbus_frame_set_transaction_id(pkt, 0x1E02); // random?
    modbus_frame_set_function(pkt, MB_FUNC_PRESET_SINGLE_REGISTER);
    modbus_frame_set_unit(pkt, 0x00);
    buff[offset++] = (base_addr>>8) & 0xFF; // addr hi
    buff[offset++] = (base_addr)    & 0xFF; // addr lo
    buff[offset++] = (num_regs>>8)  & 0xFF; // # regs hi
    buff[offset++] = (num_regs)     & 0xFF; // # regs lo
    modbus_frame_set_data(pkt, buff, offset);
  
    return 0;   
}

uint16_t
modbus_read_input_status(modbus_frame_t *pkt, uint16_t base_addr, uint16_t num_regs)
{
    char buff[128];
    uint16_t offset = 0;

    if (pkt == NULL)
        return -1;

    //modbus_frame_set_transaction_id(pkt, 0x1E02); // random?
    modbus_frame_set_function(pkt, MB_FUNC_READ_INPUT_STATUS);
    modbus_frame_set_unit(pkt, 0x00);
    buff[offset++] = (base_addr>>8) & 0xFF; // addr hi
    buff[offset++] = (base_addr)    & 0xFF; // addr lo
    buff[offset++] = (num_regs>>8)  & 0xFF; // # regs hi
    buff[offset++] = (num_regs)     & 0xFF; // # regs lo
    modbus_frame_set_data(pkt, buff, offset);
  
    return 0;   
}



//------------------------------------------------------------------------------
// Create a Modbus frame for a READ COIL STATUS command:
//------------------------------------------------------------------------------
uint16_t
modbus_read_coil_status(modbus_frame_t *pkt, uint16_t base_addr, uint16_t num_regs)
{
    char buff[128];
    uint16_t offset = 0;

    if (pkt == NULL)
        return -1;

    //modbus_frame_set_transaction_id(pkt, 0x1E02); // random?
    modbus_frame_set_function(pkt, MB_FUNC_READ_COIL_STATUS);
    modbus_frame_set_unit(pkt, 0x00);
    buff[offset++] = (base_addr>>8) & 0xFF; // addr hi
    buff[offset++] = (base_addr)    & 0xFF; // addr lo
    buff[offset++] = (num_regs>>8)  & 0xFF; // # regs hi
    buff[offset++] = (num_regs)     & 0xFF; // # regs lo
    modbus_frame_set_data(pkt, buff, offset);
  
    return 0;   
}


//------------------------------------------------------------------------------
// Create a Modbus frame for a FORCE SINGLE COIL command:
//------------------------------------------------------------------------------
uint16_t
modbus_force_single_coil(modbus_frame_t *pkt, uint16_t base_addr, uint16_t value)
{
    char buff[128];
    uint16_t offset = 0;

    if (pkt == NULL)
        return -1;

    //modbus_frame_set_transaction_id(pkt, 0x1E02); // random?
    modbus_frame_set_function(pkt, MB_FUNC_FORCE_SINGLE_COIL);
    modbus_frame_set_unit(pkt, 0x00);
    buff[offset++] = (base_addr>>8) & 0xFF; // addr hi
    buff[offset++] = (base_addr)    & 0xFF; // addr lo
    buff[offset++] = value ? 0xFF : 0x00;   // # regs hi
    buff[offset++] = 0x00;   // # regs lo: this should always be zero?
    modbus_frame_set_data(pkt, buff, offset);

    return 0;   
}

//------------------------------------------------------------------------------
// Create a Modbus frame for a FORCE SINGLE COIL command:
//------------------------------------------------------------------------------
uint16_t
modbus_diagnostics(modbus_frame_t *pkt, uint16_t subfunc, char *data, uint16_t data_len)
{
    char buff[128];
    uint16_t offset = 0, i;

    if (pkt == NULL)
        return -1;

    //modbus_frame_set_transaction_id(pkt, 0x0000); // random...
    modbus_frame_set_function(pkt, MB_FUNC_DIAGNOSTICS);
    modbus_frame_set_unit(pkt, 0x00);
    buff[offset++] = (subfunc>>8) & 0xFF; // sub function hi
    buff[offset++] = (subfunc)    & 0xFF; // sub function lo

    for (i = 0; i < data_len && offset < (uint16_t)sizeof(buff); i++)
       buff[offset++] = data[i];

    modbus_frame_set_data(pkt, buff, offset);

    return 0;   
    
}

//------------------------------------------------------------------------------
// Verify frame: check that the error flags is not set
//------------------------------------------------------------------------------
uint16_t
modbus_frame_verify(modbus_frame_t *pkt)
{
    if (!pkt)
    {
        snprintf(modbus_error_str, sizeof(modbus_error_str), "frame pouint16_ter is null.");
        return -1;
    }

    if (pkt->hdr_tcp.func_code & 0x80)
    {
        snprintf(modbus_error_str, sizeof(modbus_error_str), "Error flag set in function code.");
        return -1;
    }

    return 0;
}


//------------------------------------------------------------------------------
// Response frames:
//------------------------------------------------------------------------------
uint16_t
modbus_response_read_holding_registers(modbus_frame_t *pkt, uint16_t *value_tbl, uint16_t tbl_size)
{
    char buff[128];
    uint16_t offset = 0, i;

    if (pkt == NULL)
        return -1;

    //modbus_frame_set_transaction_id(pkt, 0x1E02); // random?
    modbus_frame_set_function(pkt, MB_FUNC_READ_HOLDING_REGISTERS);
    //modbus_frame_set_unit(pkt, 0x00);

    buff[offset++] = tbl_size * 2;
    for (i = 0; i < tbl_size; i++)
    {
        buff[offset++] = (value_tbl[i]>>8) & 0xFF; // # hi
        buff[offset++] = (value_tbl[i])    & 0xFF; // # lo    
    }

    modbus_frame_set_data(pkt, buff, offset);
  
    return 0;       
}

//------------------------------------------------------------------------------
// Error/exception frames:
//------------------------------------------------------------------------------
uint16_t
modbus_exception(modbus_frame_t *pkt, uint16_t error_code, uint16_t exception_code)
{
    char buff[128];
    uint16_t offset = 0;

    if (pkt == NULL)
        return -1;

    //modbus_frame_set_transaction_id(pkt, 0x1E02); // random?
    modbus_frame_set_function(pkt, error_code | MB_ERROR_FLAG);
    modbus_frame_set_unit(pkt, 0x00);

    buff[offset++] = exception_code;

    modbus_frame_set_data(pkt, buff, offset);

    return 0;
}


//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
// PARSING FUNCTIONS
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

uint16_t
modbus_read_holding_register_get_address(modbus_frame_t *pkt)
{
    if (pkt)
    {
        if (pkt->data_buff && pkt->data_buff_len >= 2)
        {
            return ((pkt->data_buff[0] << 8) & 0xFF00) + (pkt->data_buff[1] & 0x00FF);
        }
    }
    
    return -1;
}

uint16_t
modbus_read_holding_register_get_range(modbus_frame_t *pkt)
{
    if (pkt)
    {
        if (pkt->data_buff && pkt->data_buff_len >= 4)
        {
            return ((pkt->data_buff[2]<<8)&0xFF00) + (pkt->data_buff[3]&0x00FF);
        }
    }
    
    return -1;
}

uint16_t
modbus_error_get_exception_code(modbus_frame_t *pkt)
{
    if (pkt)
    {
        if (pkt->data_buff && pkt->data_buff_len > 0)
        {
            return pkt->data_buff[0];
        }
    }
    
    return -1;
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
// DECREPATED: ..............
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

uint16_t
modbus_frame_read_register_get_address(modbus_frame_t *pkt)
{
    if (pkt)
    {
        if (pkt->data_buff && pkt->data_buff_len > 2)
        {
            return pkt->data_buff[0] * 255 + pkt->data_buff[1];
        }
    }
    
    return -1;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
uint16_t
modbus_frame_read_register_set_address(modbus_frame_t *pkt, uint16_t address)
{
    if (pkt)
    {
        if (pkt->data_buff && pkt->data_buff_len > 2)
        {
            pkt->data_buff[0] = (address>>8) & 0xFF;
            pkt->data_buff[1] =  address     & 0xFF;
        }
    }
    
    return -1;
}


//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
uint16_t
modbus_frame_read_register_get_data(modbus_frame_t *pkt, uint16_t offset)
{
    if (pkt)
    {
        if (pkt->data_buff && pkt->data_buff_len > 2 + 2*offset)
        {
            return pkt->data_buff[2*(offset+1)] * 255 + pkt->data_buff[2*(offset+1)+1];
        }
    }
    
    return -1;
}


//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
uint16_t
modbus_frame_read_register_set_data(modbus_frame_t *pkt, char *data, uint16_t data_len)
{
    uint16_t i;

    if (pkt)
    {
        if (pkt->data_buff)
        {
            free(pkt->data_buff);
            pkt->data_buff = NULL;
        }

        pkt->data_buff_len = data_len+1;
        if ((pkt->data_buff = malloc(pkt->data_buff_len)) == NULL)
        {            
            return -1;    
        }
        modbus_frame_set_length(pkt, pkt->data_buff_len + 2);

        pkt->data_buff[0] = data_len & 0xFF;

        for (i = 0; i < data_len; i += 2)
        {
            pkt->data_buff[i+1] = data[i+1];
            pkt->data_buff[i+2] = data[i];
        }
    }
    
    return -1;
}

