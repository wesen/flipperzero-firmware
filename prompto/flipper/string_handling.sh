#!/bin/bash

# Get general flipper context
prompto get flipper/general

# Get string implementation
pinocchio catter print -f "string\.(c|h)$" -d xml furi 