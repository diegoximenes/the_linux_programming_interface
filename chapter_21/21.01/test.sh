#!/bin/bash

# prints:
# ... abort (core dumped)  ./abort 0
./abort 0

echo ""

# prints:
# sighandler 6
# ... abort (core dumped)  ./abort 1
./abort 1
