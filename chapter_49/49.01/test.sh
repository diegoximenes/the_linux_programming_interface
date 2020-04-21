#!/bin/bash

echo "testing cp_mmap" > input_file
./cp_mmap input_file output_file
echo "input_file:"
cat input_file
echo "input_file:"
cat output_file
echo "diff:"
diff input_file output_file
