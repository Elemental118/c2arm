#!/bin/bash

set -e

make
aarch64-unknown-linux-gnu-gcc -c tests/_start.s -o tests/_start.o
for input in tests/inputs/*.c; do
    input=$(basename $input .c)
    ./compiler tests/inputs/$input.c > tests/test.s
    aarch64-unknown-linux-gnu-gcc -c tests/test.s -o tests/test.o
    aarch64-unknown-linux-gnu-gcc -nostdlib tests/_start.o tests/test.o -o tests/test
    if ! hl tests/test; then 
        echo ERROR $input.c
        rm tests/test
        rm tests/test.s
        rm tests/test.o
        rm tests/_start.o
        exit 1
    fi;
done
rm tests/test
rm tests/test.s
rm tests/test.o
rm tests/_start.o
echo SUCCESS
