#!/bin/bash -e
GDB=/opt/toolchains/gcc-arm-none-eabi-8-2019-q3-update/bin/arm-none-eabi-gdb

$GDB --command=./source/config/gdb.ini -tui ./build-stm32/src_stdperiph/stm32f303xc-cmake-template.elf