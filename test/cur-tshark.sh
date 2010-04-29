#!/bin/sh

cmd='tshark'
#cmd='valgrind tshark'

$cmd -i lo -T fields \
  -e fast.tid -e fast.name -e fast.a -e fast.b \
  -e fast.c -e fast.x -e fast.y -e fast.z

