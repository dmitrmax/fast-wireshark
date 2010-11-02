#!/bin/sh

testdir=$(dirname "$0")
basedir="$testdir/.."

port=5000
template="$testdir/templates.xml"

expect=$1
pfx=$2
if [ -z "$pfx" ]
then
  pfx=output
fi

"$basedir/bin/rwcompare" \
    runner "$testdir/OpenFastPlanRunner.jar" \
    tmpl "$template" \
    expect "$expect" \
    port "$port" \
    pcap "$pfx.pcap" \
    pdml "$pfx.pdml" \
    plan "$pfx.plan"

