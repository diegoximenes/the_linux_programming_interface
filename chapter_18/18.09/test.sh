#!/bin/bash

n=10000000
new_dir="tmp"

mkdir -p "$new_dir"

printf "chdir:"
time ./chdir_performance "$n" "$new_dir"

printf "\nfchdir:"
time ./fchdir_performance "$n" "$new_dir"

rm -r "$new_dir"
