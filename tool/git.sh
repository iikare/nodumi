#!/bin/zsh

cd ..

echo "date,loc" > tool/stats.csv

for c in $(git log --children | grep commit\   | sed "s;commit ;;" | tac | xargs echo); do
   git checkout $c -- 2> /dev/null 1>&2
   e=$?
   [[ $e -ne 0 ]] && echo "skipping commit $c" && continue
   td=$(git show --no-patch --format=%ci $c)
   lct_a=$(find . -maxdepth 1 -type f -iname "*.cc" -print0 | wc -l --files0-from=- | tail -n 1 | sed 's/ total//'g)
   lct_b=$(find . -maxdepth 1 -type f -iname "*.h" -print0 | wc -l --files0-from=- | tail -n 1 | sed 's/ total//'g)
   lct_c=$(find src -type f -iname "*.cc" -print0 | wc -l --files0-from=- | tail -n 1 | sed 's/ total//'g)
   lct_d=$(find src -type f -iname "*.h" -print0 | wc -l --files0-from=- | tail -n 1 | sed 's/ total//'g)

   if [[ $lct_a == "" ]]
   then
     lct_a=0
   fi
   if [[ $lct_b == "" ]]
   then
     lct_b=0
   fi
   if [[ $lct_c == "" ]]
   then
     lct_c=0
   fi
   if [[ $lct_d == "" ]]
   then
     lct_d=0
   fi

   tl=$(echo $lct_a + $lct_b + $lct_c + $lct_d | bc)
   
   echo "$td,$tl" | tee -a tool/stats.csv
done
