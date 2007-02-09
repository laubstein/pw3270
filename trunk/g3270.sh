#!/bin/bash

. /etc/sysconfig/g3270

export Terminal3270
export Fields3270
export Cursor3270
export Selection3270
export Status3270

cd ./src
./g3270

