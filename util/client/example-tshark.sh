#!/bin/sh

cmd='tshark'
#cmd='valgrind tshark'

templatef="$(dirname "$0")/example.xml"

$cmd -i lo -o fast.port:1337 -o fast.template:"$templatef" -T pdml

