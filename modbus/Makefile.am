#-------------------------------------------------------------------------------
# Copyright (C) 2012, Robert Johansson <rob@raditex.nu>, Raditex Control AB
# All rights reserved.
#
# This file is part of rSCADA.
# http://www.rSCADA.se
# info@rscada.se
# 
#-------------------------------------------------------------------------------

PACKAGE		= @PACKAGE@
VERSION		= @VERSION@

AM_CPPFLAGS	= -I$(top_builddir) -I$(top_srcdir)

pkginclude_HEADERS = modbus.h modbus-tcp.h modbus-udp.h 
lib_LTLIBRARIES	   = libmodbus.la
libmodbus_la_SOURCES = modbus.c modbus-udp.c modbus-tcp.c

