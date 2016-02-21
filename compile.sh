#!/bin/bash

########################################
#
#        writed by august
#   with a lot of inperfect places
#
########################################

TUTDIR=$PWD
PRJDIR=$TUTDIR/project

if [ ! $# == 1 ]; then
    echo Usage:
    echo "`basename $0` <--host=arm | --host=x86>"
    exit 1
fi

echo "##########################################################"
echo    
echo "A shell script for compiling the whole project"
echo "Version 0.1 "
echo "If you come cross any bug or wish do any improvement, contact me by"
echo "august.seu@gmail.com"
echo "http://augustseu-blog.appspot.com"
echo    
echo   



if [ -n "$1" ]; then
    if [ "$1" == "--host=arm" ]; then
        PLATE="arm"
    elif [ "$1" == "--host=x86" ]; then
        PLATE="x86"
    else
        echo Not Support yet
        exit 2
    fi
else
    PLATE="x86"
fi

if [ -d $PRJDIR ]; then
    cd $TUTDIR/project
    if [ "$PLATE" == "arm" ]; then
        make distclean
        aclocal && autoconf && automake --add-missing && ./configure --host=arm-linux && make && arm-linux-strip tcpudptool
        if [ $? -eq 0 ]; then
            cp -f tcpudptool $TUTDIR/tcpudptool_arm
            echo "make tcpudptool successfully"
        else
            echo "make tcpudptool error"
            exit 3
        fi
    else
        make distclean
        aclocal && autoconf && automake --add-missing && ./configure && make && strip tcpudptool
        if [ $? -eq 0 ]; then
            cp -f tcpudptool $TUTDIR
            echo "make tcpudptool successfully"
        else
            echo "make tcpudptool error"
            exit 3
        fi
    fi
else
    echo "Please run this script in the tcpudptool dir"
    exit 4
fi


exit 0
