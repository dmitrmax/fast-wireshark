#!/bin/sh

testdir=$(dirname "$0")
basedir="$testdir/.."

port=5000
template="$testdir/templates.xml"

send=$1
shift
pfx=""
#pfx=$1
if [ -z "$pfx" ]
then
  pfx=output
fi

valgrind "$basedir/bin/rwcompare" \
    runner "$testdir/OpenFastPlanRunner.jar" \
    tmpl "$template" \
    send "$send" \
    port "$port" \
    pcap "$pfx.pcap" \
    pdml "$pfx.pdml" \
    plan "$pfx.plan" \
    "$@"

