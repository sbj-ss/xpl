#!/bin/bash

missing=0;
d=$(dirname $0);

for f in $(\
	grep -Eo '<xpl:[^ />]+' "$d/Helpers.xpl"\
	| sort \
	| uniq \
	| cut -c 6- \
	| grep -vF "$(cat $d/bootstrap-exclude.txt)" \
	| sed -Ee 's/^[a-z]/\u&/; s/-([a-z])/\u\1/g; s/$/.xpl/' \
); do
	if [ ! -f $d/../bootstrap/$f ]; then
		echo "$f is missing!"
		((missing++))
	fi
done

if [ $missing -gt 0 ]; then
	echo "$missing test(s) missing!"
	exit 1
else
	echo "All bootstrap tests present."
fi
