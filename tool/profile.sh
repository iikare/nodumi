#!/bin/zsh

C_F="\033[31;1m"
C_S="\033[34;1m"
C_E="\033[0m"

cecho()
{
    echo "$1$2$C_E"
}

# this script enables call graph visualization through gprof
# prerequisite: a binary with mcount() instrumentation via "-pg" compiler flag
# the output is a call graph in PNG form
# it will automatically be opened in firefox
if [[ $(readelf -s ./bin/nodumi | grep -E "\s(_+)?mcount\b") ]]; 
then
  echo
  echo "executing program"
  echo

  ./bin/nodumi tests/sym2_1.mid tests/bg8c.png

  echo
  echo "running gprof"

  mv gmon.out util
  gprof ./bin/nodumi util/gmon.out | gprof2dot | dot -Tpng -o util/output.png && firefox ./util/output.png

  echo
  cecho $C_S "call graph generated and opened in a firefox window"
  echo
else
  echo
  cecho $C_F "CRITICAL ERROR"
  echo
  echo "profiler script REQUIRES a build with compiler flag -pg enabled and the profiler option set in makefile"
  echo "rebuild with command \`make prof=y re\` and try again"
  echo
fi
