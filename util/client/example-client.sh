#!/bin/sh

prog="$(dirname "$0")/../bin/client"

if [ ! -e "$prog" ]
then
    printf 'Client executable does not exist: %s\n' "$prog" >&2
    exit 1
fi

"$prog"  -p 1337 \
         --int32 -5 \
         --int32 100 \
         --int32 55555 \
         --noreq --int64 2600 --req \
         --int64 4205 \
         --int64 77 \
         --ascii "Alejandro" \
         --int32 1 --int32 2 \
         --int32 26 --int32 2 \
         --bytevec 1234AB


"$prog"  -p 1337 \
         --int32 -5 \
         --int32 100 \
         --int32 55555 \
         --noreq --nop --req \
         --int64 -205 \
         --int64 77 \
         --ascii "Incremento" \
         --int32 1 --int32 5 \
         --int32 26 --int32 2 \
         --bytevec 1234AB

