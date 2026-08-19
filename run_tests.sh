#!/bin/bash

CC="${CC:-aarch64-unknown-linux-gnu-gcc}"
RUNNER="${RUNNER:-hl}"

set -e
current=""
trap '
    code=$?
    rm -f tests/test tests/test.s tests/test.o tests/_start.o
    if [ $code -ne 0 ]; then
        echo "FAILED on: $current (exit $code)"
    fi
' EXIT

make
$CC -c tests/_start.s -o tests/_start.o
for input in tests/inputs/*.c; do
    input=$(basename $input .c)
    current="$input.c"
    ./compiler tests/inputs/$input.c > tests/test.s
    $CC -c tests/test.s -o tests/test.o
    $CC -nostdlib -static -no-pie tests/_start.o tests/test.o -o tests/test
    $RUNNER tests/test
done
echo SUCCESS
