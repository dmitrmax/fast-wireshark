#!/bin/sh

cmd='tshark'
#cmd='valgrind tshark'

templatef="$(dirname "$0")/example.xml"
port=1337

$cmd -i lo -R "udp.port == $port" -o fast.port:$port -o fast.template:"$templatef" -T pdml

