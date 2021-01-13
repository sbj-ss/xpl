#!/bin/bash

processed=0
failed=0

for f in *.xpl; do
	echo "Running ${f}..."
	LD_LIBRARY_PATH=../../libxpl ../../xpl/xpl -i $f -o /tmp/${f%.*}.xml
	((processed++))
	if ! diff ${f%.*}.xml /tmp/${f%.*}.xml; then
		((failed++))
	fi
done

echo "Processed: $processed, succeeded: $(($processed - $failed)), failed: $failed"

if [ $failed -gt 0 ]; then
	exit 1
fi
