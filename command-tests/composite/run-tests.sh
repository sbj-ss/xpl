#!/bin/bash

processed=0
failed=0
errors=0
failed_files=()

for f in *.xpl; do
	local_errors=$(LD_LIBRARY_PATH=../../libxpl ../../xpl/xpl -i $f -o ${f%.*}_out.xml 2>&1 | tee >(grep -i 'error:' | wc -l) 1>&2)
	((errors+=local_errors))
	((processed++))
	if [ $local_errors -gt 0 ]; then
		((failed++))
		failed_files+=($f)
	fi
done

echo "Processed: $processed, succeeded: $(($processed - $failed)), failed: $failed (total errors: $errors)"

if [ $errors -gt 0 ]; then
	echo "To check the details, see:"
	for file in ${failed_files[@]}; do
		echo "* ${file%.*}_out.xml"
	done
	exit 1
fi
