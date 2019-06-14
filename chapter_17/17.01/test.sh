#!/bin/bash

make

echo "file" > file

setfacl -m u:diego:rw file
setfacl -m g:root:wx file
setfacl -m m:rx file

echo ""
./acl u 1000 file
echo ""
./acl g 0 file
echo ""
getfacl file

rm file
