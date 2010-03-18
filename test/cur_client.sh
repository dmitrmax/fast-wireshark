#!/bin/sh

./client --int32 -5 \
         --noreq --uint32 2600 \
         --req --ascii "why hello there"

./client --int32 -5 \
         --nop \
         --ascii "that should have incremented"

