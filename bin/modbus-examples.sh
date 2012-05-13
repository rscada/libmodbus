#-------------------------------------------------------------------------------
# Examples of how to use the MODBUS utilities from Raditex Control's libmodbus.
#

#-------------------------------------------------------------------------------
# Holding registers
#

# Read the first 10 (16bit) holding registers
modbus-tcp-holding-register 192.168.0.3 read 0 10

# read a single register at address 5
modbus-tcp-holding-register 192.168.0.3 read 5

#-------------------------------------------------------------------------------
# Read digital inputs from a MODBUS TCP unit
#

# read a single digital output (1 bit, on or off)
modbus-tcp-discrete-input 192.168.0.3 read 2

# read a range of bits starting at bit address 0
modbus-tcp-discrete-input 192.168.0.3 readrange 0

#-------------------------------------------------------------------------------
# Read, write digital outputs (coils) on a MODBUS TCP unit
#

# read the digital output state of output channel 2 (start count from 0)
modbus-tcp-discrete-output 192.168.0.3 read 2

# write a 1 (on) to digital output 3 (start counting from 0)
modbus-tcp-discrete-output 192.168.0.3 write 3 1
# read it back
modbus-tcp-discrete-output 192.168.0.3 read 3

# write a 0 (off) to digital output 3
modbus-tcp-discrete-output 192.168.0.3 write 3 0
# read it back
modbus-tcp-discrete-output 192.168.0.3 read 3

