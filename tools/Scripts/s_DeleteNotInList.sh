
cd ./data/colliders/
for file in *
  do grep -q -F "$file" "../../all_files.txt" ||  rm "$file" 
done