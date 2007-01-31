#!/bin/bash
. ../sysconfig
make DEBUG=1
LD_LIBRARY_PATH=.
./g3270
echo rc=$?
