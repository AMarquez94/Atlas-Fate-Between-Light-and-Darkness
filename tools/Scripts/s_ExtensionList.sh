#!/bin/bash

array=()
while read p; do
	filename="${p##*/}"
	array+=($filename)
done <all.txt

for f in "${array[@]}" ; do 
	echo "$f"
done 


#cd "./data/meshes"

#echo $(ls | grep -vE "$(IFS=\| && echo "${array[*]}")")

#for f in "${array[@]}" ; do 
#	echo "$f"
    #[ -f "$f" ] || echo "$f: not found"
#done 

#find ./data/meshes -maxdepth 1 -type f -regextype posix-egrep ! -regex ".*($(IFS=\| && echo "${array[*]}")).*" -print0;

# | xargs echo

#for i in "${array[@]}"
#do :
#    echo $i
#done


