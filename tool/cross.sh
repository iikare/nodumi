#!/bin/zsh

#raylib library cross-compile script

raylibdir=./dpd/raylib/src

make -j $(nproc) -C $raylibdir clean
make -j $(nproc) -C $raylibdir CC=x86_64-w64-mingw32-gcc PLATFORM=PLATFORM_DESKTOP PLATFORM_OS=WINDOWS RAYLIB_LIBTYPE=STATIC
