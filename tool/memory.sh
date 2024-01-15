#!/bin/zsh

sigint_handler() {
  rm -f *.zst
  exit
}

trap sigint_handler SIGINT
source ./tool/color.sh

# this script enables memory profiling via the "heaptrack" tool
if [[ $# -ne 1 && $# -ne 2 ]]
then
  cecho $C_F "need 1 argument, not $#"
  return -1
fi

heaptrack ./bin/nodumi $1 $2 
rm -f *.zst

