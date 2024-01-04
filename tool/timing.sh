#!/bin/zsh

sigint_handler() {
  rm -f "tool/time.txt"
  exit
}

trap sigint_handler SIGINT
source ./tool/color.sh

if [[ $# -ne 1 ]]
then
  cecho $C_F "need 1 argument, not $#"
  return -1
fi

l=0
s=0.4
for i in {1..10}
do

  timeout $s"s" ./bin/nodumi  "$1" 2>tool/time.txt
  #./bin/nodumi  "$1" > tool/time.txt & pid_n=$(echo $!)

  #sleep 1

  #kill $pid_n


  l1=$(cat tool/time.txt | tail -n 1 | sed 's/^.*time: \(.*\) seconds.*/\1/g')
  l=$(echo "$l1+$l" | bc -l)
  lf=$(echo "$l1" | awk '{printf "%f", $0}' )
  echo "$i\t: $lf"
done

l=$(echo "$l/10" | bc -l | awk '{printf "%f", $0}' )
echo "avg\t: $l : $1"

rm -f "tool/time.txt"
