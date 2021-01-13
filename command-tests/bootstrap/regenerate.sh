#!/bin/sh

for f in *.xpl; do
	LD_LIBRARY_PATH=../../libxpl ../../xpl/xpl -i $f -o ${f%.*}.xml
done
