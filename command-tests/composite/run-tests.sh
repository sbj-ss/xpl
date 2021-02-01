#!/bin/bash

processed=0
failed=0
errors=0
failed_files=()

rm -rf save/*
touch save/.keep

for f in *.xpl; do
	local_errors=$( \
		( \
			LD_LIBRARY_PATH=../../libxpl:$LD_LIBRARY_PATH ../../xpl/xpl -i $f -o out/${f%.*}.xml 2>&1; \
			echo $? > /tmp/xpl-test-result
		) | tee >( \
			grep -i 'error:' | wc -l \
		) 1>&2 \
	)
	((errors+=local_errors))
	((processed++))
	read status < /tmp/xpl-test-result
	if [ $local_errors -gt 0 -o $status -gt 0 ]; then
		((failed++))
		failed_files+=($f)
	fi
done

echo "Processed: $processed, succeeded: $(($processed - $failed)), failed: $failed (total errors: $errors)"

if [ $failed -gt 0 ]; then
	echo "To check the details, see:"
	for file in ${failed_files[@]}; do
		echo "* out/${file%.*}.xml"
	done
	exit 1
fi

unset status
