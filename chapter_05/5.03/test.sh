#!/bin/bash

num_bytes=1000000

rm -r f1 f2
./atomic_append f1 $num_bytes & ./atomic_append f1 $num_bytes
./atomic_append f2 $num_bytes -x & ./atomic_append f2 $num_bytes -x
echo 'ls -l:'
ls -l
rm f1 f2
