#!/bin/bash

processed=0
failed=0
d=$(dirname $0)

for f in $d/*.xpl; do
	echo "Running ${f}..." 1>&2
	LD_LIBRARY_PATH=$d/../../libxpl $d/../../xpl/xpl -i $f -o /tmp/$(basename ${f%.*}.xml)
	((processed++))
	if ! diff ${f%.*}.xml /tmp/$(basename ${f%.*}.xml); then
		((failed++))
	fi
done

echo "Processed: $processed, succeeded: $(($processed - $failed)), failed: $failed"

if [ $failed -gt 0 ]; then
	exit 1
fi
