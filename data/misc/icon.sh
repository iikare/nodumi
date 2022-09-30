#!/bin/zsh

# generate windows resource file 

convert ../img/icon.png -resize 256x256 icon.ico
rm -f i.res
echo "1 ICON \"icon.ico\"" >> i.rc 
x86_64-w64-mingw32-windres i.rc -O coff i.res
rm -f i.rc
