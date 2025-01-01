#!/bin/bash

make
./build/front
./build/middle
./build/backy
./spu/run.sh
rm -f in.asm in.bin m_out.txt out.txt
