#!/bin/bash

echo "file" > file
chmod u-x file
chmod u+r file
./access file
rm file
