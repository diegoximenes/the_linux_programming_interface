#!/bin/bash

page_size=$(getconf PAGESIZE)
echo "PAGESIZE=$page_size"

if (( page_size != 4096 )); then
    echo "PAGESIZE must be 4096"
    exit 1
fi

printf "\n- file_larger_than_two_pages\n"
head -c 9500 < /dev/random > file_larger_than_two_pages
printf "    - should print byte 5000 of the file: "
./mmap_signals file_larger_than_two_pages 6000 5000
printf "    - should print byte 8000 of the file: "
./mmap_signals file_larger_than_two_pages 6000 8000
printf "    - can result in SIGSEGV. It is not possible to guarantee since \
this position can be allocated by other procedure: "
./mmap_signals file_larger_than_two_pages 6000 900000

printf "\n- file_smaller_than_a_page"
head -c 2200 < /dev/random > file_smaller_than_a_page
printf "    - should print byte 2000 of the file: "
./mmap_signals file_smaller_than_a_page 8192 2000
printf "    - should print \\\0: "
./mmap_signals file_smaller_than_a_page 8192 4000
printf "    - should yield SIGBUS: "
./mmap_signals file_smaller_than_a_page 8192 5000
printf "    - can yield SIGSEGV. It is not possible to guarantee since this \
position can be allocated by other procedure: "
./mmap_signals file_smaller_than_a_page 8192 900000

rm file_larger_than_two_pages file_smaller_than_a_page
