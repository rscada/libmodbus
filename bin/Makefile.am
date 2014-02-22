#-------------------------------------------------------------------------------
# Copyright (C) 2012, Robert Johansson <rob@raditex.nu>, Raditex Control AB
# All rights reserved.
#
# This file is part of rSCADA.
# http://www.rSCADA.se
# info@rscada.se
# 
#-------------------------------------------------------------------------------

PACKAGE	= @PACKAGE@
VERSION	= @VERSION@

AM_CPPFLAGS	= -I$(top_builddir) -I$(top_srcdir)

noinst_HEADERS			= 
bin_PROGRAMS			= modbus-tcp-holding-register modbus-tcp-discrete-input \
                          modbus-tcp-discrete-output modbus-serial-holding-register

# tcp
modbus_tcp_holding_register_LDFLAGS	= -L$(top_builddir)/modbus
modbus_tcp_holding_register_LDADD	= -lmodbus 
modbus_tcp_holding_register_SOURCES	= modbus-tcp-holding-register.c 

modbus_tcp_discrete_input_LDFLAGS	= -L$(top_builddir)/modbus
modbus_tcp_discrete_input_LDADD	= -lmodbus 
modbus_tcp_discrete_input_SOURCES	= modbus-tcp-discrete-input.c

modbus_tcp_discrete_output_LDFLAGS	= -L$(top_builddir)/modbus
modbus_tcp_discrete_output_LDADD	= -lmodbus 
modbus_tcp_discrete_output_SOURCES	= modbus-tcp-discrete-output.c

# udp


# serial
modbus_serial_holding_register_LDFLAGS	= -L$(top_builddir)/modbus
modbus_serial_holding_register_LDADD	= -lmodbus 
modbus_serial_holding_register_SOURCES	= modbus-serial-holding-register.c 

