#!/bin/bash

echo "test1:"
./execlp ls ls

echo -e "\ntest2:"
./execlp /usr/bin/ls ls .. .

echo -e "\ntest3:"
./execlp inexistent_executable inexistent_executable
