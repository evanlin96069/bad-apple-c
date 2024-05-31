#!/bin/sh -e

IMAGE_DIR="resources"
IMAGE_PREFIX="out"
NUM_DIGITS=4
DELAY=0.03

for i in $(seq -f "%0${NUM_DIGITS}g" 1 6572); do
    clear
    IMAGE_FILE="${IMAGE_DIR}/${IMAGE_PREFIX}${i}.jpg.txt"
    ./format src/format.c "$IMAGE_FILE" -c
    sleep $DELAY
done

clear
