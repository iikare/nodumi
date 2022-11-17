#!/bin/zsh

#raylib library cross-compile script

read -q "e?cross compile raylib? (y/n) "
echo
if [[ $e =~ ^[Yy]$ ]]
then
  raylibdir=./dpd/raylib/src
  make -j $(nproc) -C $raylibdir clean
  make -j $(nproc) -C $raylibdir CC=x86_64-w64-mingw32-gcc PLATFORM=PLATFORM_DESKTOP PLATFORM_OS=WINDOWS RAYLIB_LIBTYPE=STATIC
fi
