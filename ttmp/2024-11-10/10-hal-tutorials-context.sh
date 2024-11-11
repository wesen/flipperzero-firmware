#!/usr/bin/env bash

pinocchio help catter 2>&1

echo ---

echo "Debug applications which use furi_hal functionality"
echo
echo

cat ttmp/2024-11-10/09-debug-by-hal.md

echo ---

echo "Example scripts to find files related to a topic"
echo
echo

pinocchio catter print prompto -d xml

echo ---

echo "FURI hal header files"
echo
echo

echo 

find targets/f7/furi_hal -type f
find targets/furi_hal_include -type f