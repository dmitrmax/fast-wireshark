#!/bin/sh

prog="$(dirname "$0")/../bin/client"

if [ ! -e "$prog" ]
then
    printf 'Client executable does not exist: %s\n' "$prog" >&2
    exit 1
fi

"$prog" -p 1337 \
        --uint32 55 \
        --uint64 22 \
        --int32  -56 \
        --int64 63 \
        --int32  4 --int64 5 \
        --ascii "hello" \
        --bytevec "EB8675309AFB" \
        --ascii  "ABCD" \
        --int32  2 --int32  3 \
        --uint32 3 \
        --int32 55 --ascii "Sequence member 1." \
        --int32 56 --ascii "Sequence member 2." \
        --int32 57 --ascii "Sequence member 3."

