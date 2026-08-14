#!/bin/bash
set -e

make
for input in tests/inputs/*.c; do
    input=$(basename $input .c)
    actual=$(./compiler tests/inputs/$input.c)
    expected=$(cat      tests/outputs/$input.txt)
    if [ "$actual" != "$expected" ]; then
        echo ERROR
        exit 1
    fi;
done
echo SUCCESS
