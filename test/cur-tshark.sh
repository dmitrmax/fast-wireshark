#!/bin/sh

tshark -i lo -T fields \
  -e fast.tid -e fast.int1 -e fast.int2 -e fast.str1 \
  -e fast.int3 -e fast.int4

