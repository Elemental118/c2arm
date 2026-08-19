#!/bin/bash

CC="${CC:-aarch64-unknown-linux-gnu-gcc}"
RUNNER="${RUNNER:-hl}"

set -e

make
$CC -c tests/_start.s -o tests/_start.o
for input in tests/inputs/*.c; do
    input=$(basename $input .c)
    ./compiler tests/inputs/$input.c > tests/test.s
    $CC -c tests/test.s -o tests/test.o
    $CC -nostdlib -static -no-pie tests/_start.o tests/test.o -o tests/test
    if ! $RUNNER tests/test; then 
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
