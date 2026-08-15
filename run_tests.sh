#!/bin/bash

make
for input in tests/inputs/*.c; do
    input=$(basename $input .c)
    actual=$(./compiler tests/inputs/$input.c 2>&1 )
    expected=$(cat      tests/outputs/$input.txt)
    if [ "$actual" != "$expected" ]; then
        echo ERROR $input.c
        exit 1
    fi;
done
echo SUCCESS
