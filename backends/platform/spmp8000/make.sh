#!/bin/bash
set -e
test -n "$LIBGAME" || export LIBGAME=../../libgame
make "$@"
objcopy -O binary scummvm scummvm.raw
$LIBGAME/../bin/mkbing scummvm.raw scummvm.bin
rm scummvm.raw
