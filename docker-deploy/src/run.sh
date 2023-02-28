#!/bin/bash
make clean
make
echo 'Start my http proxy!'
./proxy &
while true ; do continue ; done