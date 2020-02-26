#!/bin/bash
STM32_BIN=$(ls build-stm32/src*/*.bin)
OFFSET=0x8000000

st-flash --reset write "${STM32_BIN}" "${OFFSET}"