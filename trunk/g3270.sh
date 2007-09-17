#!/bin/bash

if [ -e /etc/sysconfig/g3270 ]; then
	. /etc/sysconfig/g3270
	export HOST3270_0
fi

cd ./src
./g3270 $@

