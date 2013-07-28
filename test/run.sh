#!/bin/sh

set -ue

if [ $# -ne 2 ]
then
    echo "Usage: ./run.sh image.elf golden.txt"
    exit 1
fi

readonly IMAGE=$1
readonly GOLDEN=$2

qemu-system-arm -cpu cortex-m3 -nographic -monitor null -serial null -semihosting -kernel $IMAGE | FileCheck $GOLDEN
