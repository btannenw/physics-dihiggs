#!/bin/bash

rm -rv split
mkdir -vp split

for f in $(ls *.xml);
do
  base=$(basename $f .xml)
  sed -e "/StatError/d" -e "/Channel/d" -e "/NormF/d" -e "/Data/d" -e "/DOCTYPE/d" $f > tmp.txt
  awk '{print $2}' tmp.txt |sed -e "s/Name=//" -e "s/\"//g" > presplit.txt
  awk -v RS="" '{print $0 > "split/'$base'_"$1".txt"}' presplit.txt
done

count=0
for f in $(ls split);
do
  count=$((count + $(sort split/$f | uniq | wc -l) - 1))
  echo $count
done

echo $count


