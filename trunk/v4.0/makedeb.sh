#!/bin/bash
./bootstrap.sh
./configure
dpkg-buildpackage -rfakeroot -uc -us
echo "$0 ends. rc=$?"

