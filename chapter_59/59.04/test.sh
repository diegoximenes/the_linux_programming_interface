#!/bin/bash

./server &
server_pid=$!
sleep 1

./client ::1 add key1 val1
./client ::1 add key2 val2
./client ::1 get key1
./client ::1 add key1 val1.2
./client ::1 get key1
./client ::1 delete key1
./client ::1 get key1
./client ::1 get key2
./client ::1 get key3

kill $server_pid
