#!/bin/zsh

setopt nullglob 

# remove all previous headers
find ./data -name "*.h" -type f -delete
find ./src/agh -name "*.h" -type f -delete

#build date
date=$(date +'%-d %b. %Y')
copyyear=$(date +'%y')
# overwrite file
# issue edit warning
echo "// this is an autogenerated file -- don't try to edit it!" > ./src/aghfile.h
echo "#ifndef AGHFILE_H" >> ./src/aghfile.h
echo "#define AGHFILE_H" >> ./src/aghfile.h
echo "// this is an autogenerated file -- don't try to edit it!" > ./src/agh.h

if [ $# -gt 0 ]
then

  # tally files
  convert=("./data/font/" "./data/image/")
  retype("")

  tfiles="0"

  for dir in "${convert[@]}"
  do
      ex=$(find $dir -maxdepth 1 -type f | wc -l)
      tfiles=`expr $tfiles + $ex`
  done

  #for dir in "${retype[@]}"
  #do
      #ex=$(find $dir -maxdepth 1 -type f | wc -l)
      #tfiles=`expr $tfiles + $ex`
  #done

  echo
  echo "converting $tfiles files"


  # perform conversions

  echo
  echo "binary conversion directories:"

  for dir in "${convert[@]}"
  do
      echo "$dir"
  done

  # create skeleton aghfile.h

  echo "" >> ./src/aghfile.h

  echo "#pragma once" >> ./src/aghfile.h
  echo "" >> ./src/aghfile.h
  echo "#include <string>" >> ./src/aghfile.h
  echo "#include \"asset.h\"" >> ./src/aghfile.h
  #echo "#include \"define.h\"" >> ./src/aghfile.h
  echo "#include \"agh.h\"" >> ./src/aghfile.h
  echo "" >> ./src/aghfile.h
  echo "using std::vector;" >> ./src/aghfile.h
  echo "" >> ./src/aghfile.h
  echo "#define BUILD_DATE \"$date\"" >> ./src/aghfile.h
  echo "#define COPY_YEAR \"$copyyear\"" >> ./src/aghfile.h
  echo "" >> ./src/aghfile.h
  echo "vector<asset> assetSet = {" >> ./src/aghfile.h


  # move to converting and filling out files 
  echo
  echo "list of converted files:"

  for dir in "${convert[@]}"
  do
    for file in $dir*
    do
      if [[ ! -d "$file" ]]; then
        nfile=$(basename $file)
        nfile=${nfile:0:-4}
        echo "$file → ./src/agh/$nfile.h"
        
        echo "// this is an autogenerated file -- don't try to edit it!" > "./src/agh/$nfile.h"
        echo "" >> "./src/agh/$nfile.h"
        
        echo "#pragma once" >> "./src/agh/$nfile.h"
        echo "" >> "./src/agh/$nfile.h"
        
        # since xxd doesn't like const arrays
        # NOTE: this causes compilation to fail
        #echo -n "const " >> "./src/agh/$nfile.h"
        xxd -i $file >> "./src/agh/$nfile.h"

        genfile=$(echo $file | sed 's/\./_/g' | sed 's/\//_/g')
        genfilelen=$genfile"_len"

        typedec=$(echo "${genfile:7:4}")
        
        typefin="ASSET_IMAGE"
        if [[ $typedec = "font" ]]
        then
          typefin="ASSET_FONT"
        fi

        fileid="$(echo $nfile | tr 'a-z' 'A-Z')"
        #echo "xxd generated name: $genfile, $genfilelen"
        echo "  asset($typefin, \"$fileid\",\n        $genfile, $genfilelen)," >> ./src/aghfile.h
        echo "" >> ./src/aghfile.h
      fi
    done
  done


  echo
  echo "retyped directories:"


  #for dir in $retype
  #do
    #echo $dir
  #done

  #echo
  #echo "list of converted files:"

  #for dir in "${retype[@]}"
  #do
    #for file in $dir*
    #do
      #if [[ ! -d "$file" ]]; then
        #nfile=$(basename $file | cut -f1 -d'.')

        #echo "$file → ./src/agh/$nfile.h"
        ##xxd -i $file > "./src/agh/$nfile.h"
       
        #echo "// this is an autogenerated file -- don't try to edit it!" > "./src/agh/$nfile.h"
        #echo "" >> "./src/agh/$nfile.h"
        
        #echo "#pragma once" >> "./src/agh/$nfile.h"
        #echo "" >> "./src/agh/$nfile.h"
        
        #echo "#include <string>" >> "./src/agh/$nfile.h"
        #echo "" >> "./src/agh/$nfile.h"
        #echo "using std::string;" >> "./src/agh/$nfile.h"
        #echo "" >> "./src/agh/$nfile.h"
        
        #genfile=$(echo $file | sed 's/\./_/g' | sed 's/\//_/g')
        #nl=$'\n'
        #contents=$(<$file | sed ':a;N;$!ba;s/\n/\\\\n"\n"/g') 
        ##echo $contents
        #echo "const string $genfile = ">> "./src/agh/$nfile.h"
        #echo -n "\"">> "./src/agh/$nfile.h"
        
        ## raw string data goes here
        ##contents= "$contents\""
        #echo -n $contents >> "./src/agh/$nfile.h"
        

        #echo "\";">> "./src/agh/$nfile.h"
        
        #typedec=$(echo "${genfile:7:4}")
        
        #typefin="ASSET_MAP"
        #if [[ $typedec = "shad" ]]
        #then
          #typefin="ASSET_SHADER"
        #fi

        #eventset=$typedec

        #fileid="$(echo $nfile | tr 'a-z' 'A-Z')"
        ##echo "xxd generated name: $genfile, $genfilelen"
        #echo -n "  asset($typefin, \"$fileid\", $genfile" >> ./src/aghfile.h
        
        #if [[ $typefin = "ASSET_MAP" ]]
        #then
          #echo -n ", EVENT_$fileid" >> ./src/aghfile.h
        #fi

        #echo ")," >> ./src/aghfile.h
        #echo "" >> ./src/aghfile.h
      #fi
    #done
  #done
  
  # closing bracket for definition of assetSet
  echo "};" >> ./src/aghfile.h

  echo
  echo "creating condensed header file ./src/agh.h"
 
  echo "" >> ./src/agh.h
  echo "#pragma once" >> ./src/agh.h
  echo "" >> ./src/agh.h

  for file in ./src/agh/*
  do
    ex=${file:6}

    ex="#include \"$ex\""
    
    echo $ex >> ./src/agh.h

  done


  #find ./src/data -type f -name '*.*' -printf '%p\0' | echo -

  #find ./src/data -type f -exec xxd -i {} > {}.h \;  
  #while IFS= read -r -d '' file; do
  #    ex="$file.h"
  #    xxd -i >> ex
  #done

  #for f in $(find ./src/data -print0)
  #do
  #   #xxd -u $f | grep ABCD
  #   echo $f
     #xxd -i $f
  #done#

  echo "#endif //AGHFILE_H" >> ./src/aghfile.h
  
  echo
  echo "successfully performed asset-to-header conversion"

  echo
else
  # for cleanup
  echo "" >> ./src/aghfile.h
  echo "#include <vector>" >> ./src/aghfile.h
  echo "#include \"asset.h\"" >> ./src/aghfile.h
  echo "" >> ./src/aghfile.h
  echo "#define BUILD_DATE \" \"" >> ./src/aghfile.h
  echo "#define COPY_YEAR \" \"" >> ./src/aghfile.h
  echo "" >> ./src/aghfile.h
  echo "vector<asset> assetSet = {};" >> ./src/aghfile.h
  echo "#endif //AGHFILE_H" >> ./src/aghfile.h
fi
