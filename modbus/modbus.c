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
// Allocate a new modbus-tcp packet_t data structure
//------------------------------------------------------------------------------
modbus_packet_t *
modbus_packet_new()
{
    modbus_packet_t *pkt;
    
    if ((pkt = malloc(sizeof(modbus_packet_t))) != NULL)
    {
        pkt->hdr.transaction1 = 0x00;
        pkt->hdr.transaction2 = 0x00;
        pkt->hdr.protocol1 = 0x00;
        pkt->hdr.protocol2 = 0x00;
        pkt->hdr.length1 = 0x00;
        pkt->hdr.length2 = 0x00;
        pkt->hdr.unit = 0xFF;
        pkt->hdr.func_code = 0x00;
        
        pkt->data_buff     = NULL;
        pkt->data_buff_len = 0;
        return pkt;
    }

    return NULL;
}

//------------------------------------------------------------------------------
// Free allocated data associated with the modbus-tcp packet_t data structure
//------------------------------------------------------------------------------
void
modbus_packet_free(modbus_packet_t *pkt)
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
// dump packet in a human-readable form
//------------------------------------------------------------------------------
void
modbus_packet_print(modbus_packet_t *pkt)
{
    uint16_t i;
    
    if (pkt)
    {
        printf("MODBUS: packet DUMP\n");
        printf("MODBUS: transaction1  = %.2X\n", pkt->hdr.transaction1 & 0xFF);
        printf("MODBUS: transaction2  = %.2X\n", pkt->hdr.transaction2 & 0xFF);
        printf("MODBUS: protocol1     = %.2X\n", pkt->hdr.protocol1 & 0xFF);
        printf("MODBUS: protocol2     = %.2X\n", pkt->hdr.protocol2 & 0xFF);
        printf("MODBUS: length1       = %.2X\n", pkt->hdr.length1   & 0xFF);
        printf("MODBUS: length2       = %.2X\n", pkt->hdr.length2   & 0xFF);
        printf("MODBUS: unit id       = %.2X\n", pkt->hdr.unit      & 0xFF);
        printf("MODBUS: function code = %.2X\n", pkt->hdr.func_code & 0xFF);
        
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


//------------------------------------------------------------------------------
// Parse binary data and setup packet data structure
//------------------------------------------------------------------------------
uint16_t
modbus_header_parse(modbus_packet_t *pkt, char *data, size_t data_size)
{   
    if (data_size < MODBUS_HEADER_LENGTH)
    {
        // too small data. it cannot fit a proper modbus packet
        return -1;
    }
    
    memcpy((void *)&pkt->hdr, (void *)data, MODBUS_HEADER_LENGTH);
       
    return 0;
}

//------------------------------------------------------------------------------
// Parse binary data and setup packet_t data structure
//------------------------------------------------------------------------------
uint16_t
modbus_packet_parse(modbus_packet_t *pkt, char *data, size_t data_size)
{
    uint16_t len, i, idx;
    
    if (data_size < MODBUS_HEADER_LENGTH)
    {
        // too small data. it cannot fit a proper modbus packet
        printf("data_size(%zu) < MODBUS_HEADER_LENGTH\n", data_size);
        return -1;
    }
    
    idx = 0;
    
    pkt->hdr.transaction1 = data[idx++];
    pkt->hdr.transaction2 = data[idx++];
    pkt->hdr.protocol1    = data[idx++];
    pkt->hdr.protocol2    = data[idx++];     
    pkt->hdr.length1      = data[idx++];
    pkt->hdr.length2      = data[idx++];
    pkt->hdr.unit         = data[idx++];
    pkt->hdr.func_code    = data[idx++];
    
    //if (modbus_header_parse(pkt, data, data_size) == -1)
    //{
    //    return -1;
    //}
    //idx = MODBUS_HEADER_LENGTH
        
    //len = (pkt->hdr.length2 + pkt->hdr.length1 * 256) - 2;
    len = modbus_packet_get_length(pkt) - 2;  


    if ((size_t)(MODBUS_HEADER_LENGTH + len) != data_size)
    {
        printf("%s: packet length mismatch (data_size = %zu, expected = %zu)\n", __PRETTY_FUNCTION__, data_size, (size_t)(MODBUS_HEADER_LENGTH + len));
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
// Generate binary packet_t from packet_t data structure
//------------------------------------------------------------------------------
uint16_t
modbus_packet_pack(modbus_packet_t *pkt, char *data_buff, size_t data_buff_size)
{
    uint16_t pkt_len, idx, i;
    
    if (pkt == NULL)
    {
        return -1;
    }
    
    pkt_len = MODBUS_HEADER_LENGTH + pkt->data_buff_len;
    
    if ((size_t)pkt_len > data_buff_size)
    {
        return -1;
    }

    idx = 0;
    
    data_buff[idx++] = pkt->hdr.transaction1;
    data_buff[idx++] = pkt->hdr.transaction2;    
    data_buff[idx++] = pkt->hdr.protocol1;
    data_buff[idx++] = pkt->hdr.protocol2;     
    data_buff[idx++] = pkt->hdr.length1;
    data_buff[idx++] = pkt->hdr.length2;
    data_buff[idx++] = pkt->hdr.unit;
    data_buff[idx++] = pkt->hdr.func_code;
    
    for (i = 0; i < pkt->data_buff_len; i++)
    {
        data_buff[idx++] = pkt->data_buff[i];
    }
    
    return idx;
}

//------------------------------------------------------------------------------
// 
//------------------------------------------------------------------------------
uint16_t
modbus_packet_get_data_size(modbus_packet_t *pkt)
{
    if (pkt)
    {
        return pkt->hdr.length1 * 255 + pkt->hdr.length2 - 2;
    }

    return -1;
}

//------------------------------------------------------------------------------
// 
//------------------------------------------------------------------------------
uint16_t
modbus_packet_get_transaction_id(modbus_packet_t *pkt)
{
    if (pkt)
    {
        return pkt->hdr.transaction1 * 255 + pkt->hdr.transaction2;
    }

    return -1;
}

//------------------------------------------------------------------------------
// 
//------------------------------------------------------------------------------
uint8_t
modbus_packet_get_unit(modbus_packet_t *pkt)
{
    if (pkt)
    {
        return pkt->hdr.unit;
    }
    return -1;
}

//------------------------------------------------------------------------------
// 
//------------------------------------------------------------------------------
uint8_t
modbus_packet_get_function(modbus_packet_t *pkt)
{
    if (pkt)
    {
        return pkt->hdr.func_code;
    }
    return -1;
}

//------------------------------------------------------------------------------
// 
//------------------------------------------------------------------------------
uint16_t
modbus_packet_get_length(modbus_packet_t *pkt)
{
    if (pkt)
    {
        return  pkt->hdr.length1 * 255 + pkt->hdr.length2;
    }
    
    return -1;
}

//------------------------------------------------------------------------------
// 
//------------------------------------------------------------------------------
uint16_t
modbus_packet_set_data(modbus_packet_t *pkt, char *data, size_t data_size)
{   
    if ((pkt->data_buff = malloc(data_size)) == NULL)
    {
        printf("%s: failed to allocate data_buff (%zu)\n", __PRETTY_FUNCTION__, data_size);
        return -1;
    }  
    
    memcpy((void *)pkt->data_buff, (void *)data, data_size);
    pkt->data_buff_len = data_size;
    
    modbus_packet_set_length(pkt, data_size + 2);
    return 0;    
}

//------------------------------------------------------------------------------
// 
//------------------------------------------------------------------------------
uint16_t
modbus_packet_set_unit(modbus_packet_t *pkt, char val)
{
    if (pkt)
    {
        pkt->hdr.unit = val;
    }
    
    return 0;
}


//------------------------------------------------------------------------------
// 
//------------------------------------------------------------------------------
uint16_t
modbus_packet_set_function(modbus_packet_t *pkt, char val)
{
    if (pkt)
    {
        pkt->hdr.func_code = val;
    }
    
    return 0;
}

//------------------------------------------------------------------------------
// 
//------------------------------------------------------------------------------
uint16_t
modbus_packet_set_length(modbus_packet_t *pkt, uint16_t len)
{
    if (pkt)
    {
        pkt->hdr.length1 = (len>>8) & 0xff;
        pkt->hdr.length2 =  len     & 0xff;
    }
    
    return 0;
}

//------------------------------------------------------------------------------
// 
//------------------------------------------------------------------------------
uint16_t
modbus_packet_set_transaction_id(modbus_packet_t *pkt, uint16_t id)
{
    if (pkt)
    {
        pkt->hdr.transaction1 = (id>>8) & 0xff;
        pkt->hdr.transaction2 =  id     & 0xff;
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
modbus_packet_data_num_bytes(modbus_packet_t *pkt)
{
    if (pkt == NULL || pkt->data_buff == NULL || pkt->data_buff_len == 0)
        return -1;

    return pkt->data_buff[0];
}

//
// get n'th 8 bit register from the data
//
uint16_t
modbus_packet_reg8_get(modbus_packet_t *pkt, uint16_t n)
{
    if (pkt == NULL || pkt->data_buff == NULL || pkt->data_buff_len < n+2)
        return -1;

    return (uint16_t)pkt->data_buff[n + 1];    
}

//
// get n'th 16 bit register from the data
//
uint16_t
modbus_packet_reg16_get(modbus_packet_t *pkt, uint16_t n)
{
    uint16_t val;

    if (pkt == NULL || pkt->data_buff == NULL || pkt->data_buff_len < 2*(n+1)+1)
        return -1;

    val = (uint16_t)((pkt->data_buff[2*n + 1] & 0xFF) << 8) +
          (uint16_t)(pkt->data_buff[2*n + 2] & 0xFF);

    return val;
}

uint16_t
modbus_packet_data_decode16(modbus_packet_t *pkt, uint16_t offset)
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
modbus_packet_reg32_get(modbus_packet_t *pkt, uint16_t n)
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
modbus_packet_data_bit_get(modbus_packet_t *pkt, uint16_t n)
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
modbus_read_holding_registers(modbus_packet_t *pkt, uint16_t base_addr, uint16_t num_regs)
{
    char buff[128];
    uint16_t offset = 0;

    if (pkt == NULL)
        return -1;

    //modbus_packet_set_transaction_id(pkt, 0x1E02); // random?
    modbus_packet_set_function(pkt, MB_FUNC_READ_HOLDING_REGISTER);
    modbus_packet_set_unit(pkt, 0x00);
    buff[offset++] = (base_addr>>8) & 0xFF; // addr hi
    buff[offset++] = (base_addr)    & 0xFF; // addr lo
    buff[offset++] = (num_regs>>8)  & 0xFF; // # regs hi
    buff[offset++] = (num_regs)     & 0xFF; // # regs lo
    modbus_packet_set_data(pkt, buff, offset);
  
    return 0;   
}


uint16_t
modbus_preset_single_register(modbus_packet_t *pkt, uint16_t base_addr, uint16_t num_regs)
{
    char buff[128];
    uint16_t offset = 0;

    if (pkt == NULL)
        return -1;

    //modbus_packet_set_transaction_id(pkt, 0x1E02); // random?
    modbus_packet_set_function(pkt, MB_FUNC_PRESET_SINGLE_REGISTER);
    modbus_packet_set_unit(pkt, 0x00);
    buff[offset++] = (base_addr>>8) & 0xFF; // addr hi
    buff[offset++] = (base_addr)    & 0xFF; // addr lo
    buff[offset++] = (num_regs>>8)  & 0xFF; // # regs hi
    buff[offset++] = (num_regs)     & 0xFF; // # regs lo
    modbus_packet_set_data(pkt, buff, offset);
  
    return 0;   
}

uint16_t
modbus_read_input_status(modbus_packet_t *pkt, uint16_t base_addr, uint16_t num_regs)
{
    char buff[128];
    uint16_t offset = 0;

    if (pkt == NULL)
        return -1;

    //modbus_packet_set_transaction_id(pkt, 0x1E02); // random?
    modbus_packet_set_function(pkt, MB_FUNC_READ_INPUT_STATUS);
    modbus_packet_set_unit(pkt, 0x00);
    buff[offset++] = (base_addr>>8) & 0xFF; // addr hi
    buff[offset++] = (base_addr)    & 0xFF; // addr lo
    buff[offset++] = (num_regs>>8)  & 0xFF; // # regs hi
    buff[offset++] = (num_regs)     & 0xFF; // # regs lo
    modbus_packet_set_data(pkt, buff, offset);
  
    return 0;   
}



//------------------------------------------------------------------------------
// Create a Modbus packet for a READ COIL STATUS command:
//------------------------------------------------------------------------------
uint16_t
modbus_read_coil_status(modbus_packet_t *pkt, uint16_t base_addr, uint16_t num_regs)
{
    char buff[128];
    uint16_t offset = 0;

    if (pkt == NULL)
        return -1;

    //modbus_packet_set_transaction_id(pkt, 0x1E02); // random?
    modbus_packet_set_function(pkt, MB_FUNC_READ_COIL_STATUS);
    modbus_packet_set_unit(pkt, 0x00);
    buff[offset++] = (base_addr>>8) & 0xFF; // addr hi
    buff[offset++] = (base_addr)    & 0xFF; // addr lo
    buff[offset++] = (num_regs>>8)  & 0xFF; // # regs hi
    buff[offset++] = (num_regs)     & 0xFF; // # regs lo
    modbus_packet_set_data(pkt, buff, offset);
  
    return 0;   
}


//------------------------------------------------------------------------------
// Create a Modbus packet for a FORCE SINGLE COIL command:
//------------------------------------------------------------------------------
uint16_t
modbus_force_single_coil(modbus_packet_t *pkt, uint16_t base_addr, uint16_t value)
{
    char buff[128];
    uint16_t offset = 0;

    if (pkt == NULL)
        return -1;

    //modbus_packet_set_transaction_id(pkt, 0x1E02); // random?
    modbus_packet_set_function(pkt, MB_FUNC_FORCE_SINGLE_COIL);
    modbus_packet_set_unit(pkt, 0x00);
    buff[offset++] = (base_addr>>8) & 0xFF; // addr hi
    buff[offset++] = (base_addr)    & 0xFF; // addr lo
    buff[offset++] = value ? 0xFF : 0x00;   // # regs hi
    buff[offset++] = 0x00;   // # regs lo: this should always be zero?
    modbus_packet_set_data(pkt, buff, offset);

    return 0;   
}

//------------------------------------------------------------------------------
// Create a Modbus packet for a FORCE SINGLE COIL command:
//------------------------------------------------------------------------------
uint16_t
modbus_diagnostics(modbus_packet_t *pkt, uint16_t subfunc, char *data, uint16_t data_len)
{
    char buff[128];
    uint16_t offset = 0, i;

    if (pkt == NULL)
        return -1;

    //modbus_packet_set_transaction_id(pkt, 0x0000); // random...
    modbus_packet_set_function(pkt, MB_FUNC_DIAGNOSTICS);
    modbus_packet_set_unit(pkt, 0x00);
    buff[offset++] = (subfunc>>8) & 0xFF; // sub function hi
    buff[offset++] = (subfunc)    & 0xFF; // sub function lo

    for (i = 0; i < data_len && offset < (uint16_t)sizeof(buff); i++)
       buff[offset++] = data[i];

    modbus_packet_set_data(pkt, buff, offset);

    return 0;   
    
}

//------------------------------------------------------------------------------
// Verify packet: check that the error flags is not set
//------------------------------------------------------------------------------
uint16_t
modbus_packet_verify(modbus_packet_t *pkt)
{
    if (!pkt)
    {
        snprintf(modbus_error_str, sizeof(modbus_error_str), "Packet pouint16_ter is null.");
        return -1;
    }

    if (pkt->hdr.func_code & 0x80)
    {
        snprintf(modbus_error_str, sizeof(modbus_error_str), "Error flag set in function code.");
        return -1;
    }

    return 0;
}


//------------------------------------------------------------------------------
// Response packets:
//------------------------------------------------------------------------------
uint16_t
modbus_response_read_holding_registers(modbus_packet_t *pkt, uint16_t *value_tbl, uint16_t tbl_size)
{
    char buff[128];
    uint16_t offset = 0, i;

    if (pkt == NULL)
        return -1;

    //modbus_packet_set_transaction_id(pkt, 0x1E02); // random?
    modbus_packet_set_function(pkt, MB_FUNC_READ_HOLDING_REGISTER);
    //modbus_packet_set_unit(pkt, 0x00);

    buff[offset++] = tbl_size * 2;
    for (i = 0; i < tbl_size; i++)
    {
        buff[offset++] = (value_tbl[i]>>8) & 0xFF; // # hi
        buff[offset++] = (value_tbl[i])    & 0xFF; // # lo    
    }

    modbus_packet_set_data(pkt, buff, offset);
  
    return 0;       
}

//------------------------------------------------------------------------------
// Error/exception packets:
//------------------------------------------------------------------------------
uint16_t
modbus_exception(modbus_packet_t *pkt, uint16_t error_code, uint16_t exception_code)
{
    char buff[128];
    uint16_t offset = 0;

    if (pkt == NULL)
        return -1;

    //modbus_packet_set_transaction_id(pkt, 0x1E02); // random?
    modbus_packet_set_function(pkt, error_code | MB_ERROR_FLAG);
    modbus_packet_set_unit(pkt, 0x00);

    buff[offset++] = exception_code;

    modbus_packet_set_data(pkt, buff, offset);

    return 0;
}


//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
// PARSING FUNCTIONS
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

uint16_t
modbus_read_holding_register_get_address(modbus_packet_t *pkt)
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
modbus_read_holding_register_get_range(modbus_packet_t *pkt)
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
modbus_error_get_exception_code(modbus_packet_t *pkt)
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
modbus_packet_read_register_get_address(modbus_packet_t *pkt)
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
modbus_packet_read_register_set_address(modbus_packet_t *pkt, uint16_t address)
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
modbus_packet_read_register_get_data(modbus_packet_t *pkt, uint16_t offset)
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
modbus_packet_read_register_set_data(modbus_packet_t *pkt, char *data, uint16_t data_len)
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
        modbus_packet_set_length(pkt, pkt->data_buff_len + 2);

        pkt->data_buff[0] = data_len & 0xFF;

        for (i = 0; i < data_len; i += 2)
        {
            pkt->data_buff[i+1] = data[i+1];
            pkt->data_buff[i+2] = data[i];
        }
    }
    
    return -1;
}

