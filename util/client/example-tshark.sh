#!/bin/sh

cmd='tshark'
#cmd='valgrind tshark'

templatef="$(dirname "$0")/example.xml"

$cmd -i lo -o fast.port:1337 -o fast.template:"$templatef" \
  -T fields \
  -e fast.tid -e fast.a -e fast.b -e fast.c \
  -e fast.x -e fast.y -e fast.z \
  -e fast.name \
  -e fast.decval1 -e fast.decval2 -e fast.byte1

