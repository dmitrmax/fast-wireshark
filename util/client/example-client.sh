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
        --int32  56 \
        --int64 -51 \
        --int32  55 --int64 -3 \
        --ascii "hello"

