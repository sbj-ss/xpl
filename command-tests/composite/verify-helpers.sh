#!/bin/bash

missing=0;

for f in $(\
	grep -Eo '<xpl:[^ />]+' "$(dirname $0)/Helpers.xpl"\
	| sort \
	| uniq \
	| cut -c 6- \
	| grep -vF "$(cat $(dirname $0)/bootstrap-exclude.txt)" \
	| sed -Ee 's/^[a-z]/\u&/; s/-([a-z])/\u\1/g; s/$/.xpl/' \
); do
	if [ ! -f ../bootstrap/$f ]; then
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
