#!/bin/zsh

echo
if [[ $# -ne 1 ]]
then
  echo "need 1 argument, not $#"
  return -1
fi

function clear_cache() {
  echo
  echo "discrepancy found in cached object files - purging cache"
  echo
  rm -f build/*.o
}

if [[ "$1" = "win" || "$1" = "linux" ]] 
then
  objcount=$(find build -type f -name '*.o' | wc -l)

  if [[ "$1" = "win" ]] 
  then
    #expecting objects of type COFF
    coffcount=$(find build -type f -name '*.o' -exec sh -c 'file {} | grep COFF' \; | wc -l)
    if (( coffcount != objcount ))
    then
      # discrepancy found
      clear_cache
    fi

  else 
    # expecting objects of type ELF
    elfcount=$(find build -type f -name '*.o' -exec sh -c 'file {} | grep ELF' \; | wc -l)
    if (( elfcount != objcount ))
    then
      # discrepancy found
      clear_cache
    fi
  fi

else
    echo "invalid argument: $1"
fi


