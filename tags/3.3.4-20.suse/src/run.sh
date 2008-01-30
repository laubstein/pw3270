#!/bin/bash

if [ -e /etc/sysconfig/g3270 ]; then
   . /etc/sysconfig/g3270
else
   . ../sysconfig
fi

export Terminal3270
export Fields3270
export Cursor3270
export Selection3270
export Status3270
export HOST3270_0

make DEBUG=1
LD_LIBRARY_PATH=. ./g3270
echo rc=$?
