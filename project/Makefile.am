###########################################
#				make.am                   #
#										  #
#			  writed by august			  #	
###########################################
#version 0.1

if DEBUG
DEBUGFLAG=-O0 -g -Wall
else
DEBUGFLAG=-O2
endif
BUILDPF = __POSIX__

TUT_DIR =  ..

TUT_BIN_NAME = tcpudptool
TUT_LIB_NAME = tutsppt

TUT_HEAD_DIR = $(TUT_DIR)/src/include
TUT_SRC_DIR  = $(TUT_DIR)/src
TUT_LIB_DIR	 = $(TUT_DIR)/lib

#my bin need a static lib called libtutspptsup
AM_CFLAGS	= -I$(TUT_HEAD_DIR) -D$(BUILDARCH) -D$(BUILDPF) $(DEBUGFLAG) 
AM_LDFLAGS 	= 
# the liberary is called hellosup
# following sentences is used when users process 'make install'
libtutspptdir = /usr/lib
libtutsppt_includedir = /usr/local/tut/include
libtutsppt_include_HEADERS = $(TUT_HEAD_DIR)/common.h

libtutsppt_LIBRARIES = libtutsppt.a
# list sources
if  ARMLINUX
AM_CFLAGS += -I$(TUT_DIR)/ncurses-arm/include
else
AM_CFLAGS +=
endif
libtutsppt_a_SOURCES = $(TUT_SRC_DIR)/rxtx/rxtx.c    \
				        $(TUT_SRC_DIR)/rxtx/rxtx.h   \
			            $(TUT_SRC_DIR)/tthread/tthread.c \
						$(TUT_SRC_DIR)/tthread/tthread.h \
						$(TUT_HEAD_DIR)/common.h \
						$(TUT_HEAD_DIR)/types.h 


						

#the bin
bin_PROGRAMS = tcpudptool
#list sources 
tcpudptool_SOURCES = $(TUT_SRC_DIR)/main.c	\
					  $(TUT_SRC_DIR)/TCPSvr.c	\
					  $(TUT_SRC_DIR)/TCPSvr.h    \
					  $(TUT_SRC_DIR)/TCPClt.c    \
					  $(TUT_SRC_DIR)/TCPClt.h    \
					  $(TUT_HEAD_DIR)/common.h   \
					  $(TUT_HEAD_DIR)/types.h
					  

tcpudptool_CFLAGS 	= $(AM_CFLAGS) 
tcpudptool_LDFLAGS 	= -L.
tcpudptool_LDADD 	= -lpthread -lrt -lcurses

# as described in the make.am, we should examine varieties platforms
if  ARMLINUX
tcpudptool_CFLAGS += -static -DARCH_IS_ARM -I$(TUT_DIR)/ncurses-arm/include 
tcpudptool_LDADD +=  -L$(TUT_LIB_DIR)/ -L$(TUT_DIR)/ncurses-arm/lib -ltutsppt
else
tcpudptool_LDADD += -L$(TUT_LIB_DIR)/ -ltutsppt 
endif
