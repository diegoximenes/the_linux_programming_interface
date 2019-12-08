#!/bin/bash

# same result
./realpath realpath
./realpath ./realpath
./realpath ./././realpath
./realpath ../../chapter_18/././18.03/realpath

# /
./realpath ../../../../../../../..
./realpath /..
./realpath /.

# symlink
./realpath ../../../../../../../../../../bin/python

# error
./realpath ./realpath/..
./realpath ./file_that_not_exists
