#!/bin/bash

gcc -Wall ../src/dbgmain.c ../src/TCPClt.c ../src/tthread/tthread.c ../src/rxtx/rxtx.c -lcurses -lpthread -o binclient
