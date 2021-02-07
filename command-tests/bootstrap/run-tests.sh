#!/bin/bash

processed=0
failed=0
d=$(dirname $0)
xpl=${XPL:-xpl}

for f in $d/*.xpl; do
	echo "Running ${f}..." 1>&2
	if ! LD_LIBRARY_PATH=$d/../../libxpl:$LD_LIBRARY_PATH $RUNNER $d/../../xpl/$xpl -i $f -o /tmp/$(basename ${f%.*}.xml); then
		((failed++))
	else
		((processed++))
		if ! diff --ignore-all-space ${f%.*}.xml /tmp/$(basename ${f%.*}.xml); then
			((failed++))
		fi
	fi
done

echo "Processed: $processed, succeeded: $(($processed - $failed)), failed: $failed"

if [ $failed -gt 0 ]; then
	exit 1
fi
