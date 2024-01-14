#!/bin/zsh

source ./tool/color.sh

# this script enables profiling via the "perf" tool
if [[ $# -ne 1 ]]
then
  cecho $C_F "need 1 argument, not $#"
  return -1
fi


perf record -g -F 999 ./bin/nodumi $1
perf script -F +pid > /tmp/test.perf
rm -f perf.data
