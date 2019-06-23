#!/bin/bash

mkdir ./tmp
mkdir ./tmp/tmp1
mkdir ./tmp/tmp2

./monitor_events ./tmp &
sleep 1

echo "file" > ./tmp/file1
echo "file" > ./tmp/tmp1/file2
mkdir ./tmp/tmp1/tmp3/
echo "file" > ./tmp/tmp1/tmp3/file3
rm ./tmp/tmp1/tmp3/file3
echo "file" > ./tmp/tmp2/file4

rm -r ./tmp
