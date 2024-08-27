#！/bin/bash

set -e

rm -rf `pwd`/build/*
cd `pwd`/bulid &&
    cmake .. &&
    make
cd..
cp -r `pwd`/src/include `pwd`/lib