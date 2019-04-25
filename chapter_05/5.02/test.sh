#!/bin/bash

file='test.txt'

echo 'first line' > "$file"
./o_append "$file"
echo "cat $file:"
cat "$file"
rm "$file"
