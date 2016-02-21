#!/bin/bash

gcc -Wall ../src/dbgmain.c  ../src/TCPSvr.c ../src/tthread/tthread.c ../src/rxtx/rxtx.c -lcurses -lpthread -o binserver
