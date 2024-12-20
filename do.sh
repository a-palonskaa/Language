#!/bin/bash

make clean
make libs
make prog
./build/diff
cd ./backend
make clean
make
../build/backy
cd ../
cd ./spu
./run.sh
cd ../
