#!/bin/sh -e

gcc src/format.c -o format

if [ $# -eq 1 ]; then
    ./format src/format.c "$1" > out.c
    rm -f out
    gcc out.c -w -o out
fi
