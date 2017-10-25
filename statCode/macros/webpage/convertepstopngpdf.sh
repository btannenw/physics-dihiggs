#!/bin/bash

### Garoe Gonzalez 
### Converts all eps in a directory into png and pdf.
### Skip conversion if outfile is newer than infile.
### Do for all sub-dirs.

files=$(find ./ -regex ".*eps" | grep -v shapes)

for f in $files; do
    echo $f
    # png
    fout=${f/.eps/.png}
    if [ $f -nt $fout ]; then
	convert $f $fout &
	fout=${f/.eps/.pdf}
	epstopdf $f &
	while [ $(ps -a | grep -c convert) -gt "20" ]; do
	    echo "wait"
	    sleep 1
	done
    fi
done
