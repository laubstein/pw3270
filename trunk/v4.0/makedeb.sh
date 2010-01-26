#!/bin/bash
dpkg-buildpackage -rfakeroot -uc -us
echo "$0 ends. rc=$?"

