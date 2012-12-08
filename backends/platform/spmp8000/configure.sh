#!/bin/sh 
test -z "$LIBGAME" && export LIBGAME=../../libgame
./configure \
    --host=spmp8000 \
    --disable-all-engines \
    --enable-engine=scumm \
    --enable-verbose-build \
    --enable-release \
    --disable-scalers \
    --disable-translation "$@" \
    --enable-zlib --with-zlib-prefix=$LIBGAME/../3rdparty \
    --enable-mad --with-mad-prefix=$LIBGAME/../3rdparty \
    --enable-tremor --with-tremor-prefix=$LIBGAME/../3rdparty
