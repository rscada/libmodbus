dnl ----------------------------------------------------------------------------
dnl Copyright (C) 2012, Robert Johansson <rob@raditex.nu>, Raditex Control AB
dnl All rights reserved.
dnl
dnl This file is part of rSCADA.
dnl http://www.rSCADA.se
dnl info@rscada.se
dnl 
dnl ----------------------------------------------------------------------------

AC_INIT([modbus], [0.3.0], [info@rscada.se], [modbus], [http://www.rscada.se/libmodbus/])
AM_INIT_AUTOMAKE([-Wall -Werror foreign])

AC_CONFIG_MACRO_DIR([m4])

LT_INIT

AM_PROG_LIBTOOL

LDFLAGS="$LDFLAGS -version-info 0:3:0"

dnl ----------------------
dnl 
AC_PROG_CC

AC_CONFIG_HEADERS([config.h])
AC_CONFIG_FILES([Makefile modbus/Makefile bin/Makefile libmodbus.pc])
AC_OUTPUT


echo \
"----------------------------------------------------------
Configuration:

	Source location:	${srcdir}
	Compile:		    ${CC}
	Compiler flags:		${CFLAGS}
	Linker flags:		${LDFLAGS}
	Host system type:	${host}
	Install path:		${prefix}

	See config.h for further configuration.
----------------------------------------------------------"
