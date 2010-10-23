#!/bin/sh

port=5000
template=test/templates.xml

expect=$1
pfx=$2
if [ -z "$pfx" ]
then
  pfx=output
fi

./bin/rwcompare \
    runner "test/OpenFastPlanRunner.jar" \
    tmpl "$template" \
    expect "$expect" \
    port "$port" \
    pcap "$pfx.pcap" \
    pdml "$pfx.pdml" \
    plan "$pfx.plan"

