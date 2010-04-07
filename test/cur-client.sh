#!/bin/sh

./client --int32 -5 \
         --noreq --uint32 2600 \
         --req --ascii "why hello there" \
         --int64 -52584205 \
         --uint64 55555555555 \
         --bytevec 1234AB \
         --int32 -2 \
         --int32 314


./client --int32 314 \
         --nop \
         --ascii "that should have incremented" \
         --int64 -52584205 \
         --uint64 55555555555 \
         --bytevec FE4321 \
         --int32 2 \
         --int32 26

