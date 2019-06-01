#!/bin/bash

N=500000

mkdir ./tmp
# 9 sec
./fs_performance "$N" ./tmp 0
# 19 sec
./fs_performance "$N" ./tmp 1
rm -r ./tmp
