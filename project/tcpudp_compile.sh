#!/bin/bash

gcc -Wall ../src/main.c ../src/TCPClt.c ../src/TCPSvr.c ../src/tthread/tthread.c ../src/rxtx/rxtx.c -lcurses -lpthread -o ok
