#!/bin/bash

# blockdev --getbsz /dev/sda5
block_size=4096

# create a file with a hole
dd if=/dev/urandom bs=$block_size count=5 of=file_with_holes status=none
dd if=/dev/urandom bs=$block_size count=5 seek=100 of=file_with_holes \
    status=none

# create a file with the same length of the previous file but without holes
dd if=/dev/urandom bs=$block_size count=105 of=file_without_holes status=none

# copy
./cp_holes file_with_holes cp_file_with_holes cp_naive_file_with_holes

# it is possible to observe that file_with_holes and cp_file_with_holes have
# 40 blocks while file_without_holes has 420 blocks. Also, all files have the
# same length 430080.
echo 'ls -ls:'
ls -ls cp_naive_file_with_holes file_with_holes file_without_holes \
    cp_file_with_holes

# files have equal content
echo 'diff:'
diff file_with_holes cp_file_with_holes

rm cp_file_with_holes cp_naive_file_with_holes \
    file_with_holes file_without_holes
