#!/bin/sh

pfx=$2
if [ -z "$pfx" ]
then
  pfx=output
fi

./bin/rwcompare \
    runner test/OpenFastPlanRunner.jar \
    tmpl test/templates.xml \
    expect "$1" \
    pcap "$pfx.pcap" \
    pdml "$pfx.pdml" \
    plan "$pfx.plan"

