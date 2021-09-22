#!/bin/bash

# temporary file for compiler output
codout="$(mktemp)"

# names of test scripts stripped of extentions
tests="$(find ./correct_cod/* | sed -E 's/\.in|\.stdout|\.stdin//g' | sort -u)"

for test in $tests; do

	# dump asm to temporary file
	./compiler "$test.in" "$codout"

	if   [ "$?" = 2 ]; then
		echo "SEGV $test" 1>&2
	elif [ "$?" = 3 ]; then
		echo "SEME $test" 1>&2
	else

		# load expected output
		orig="$(cat "$test.stdout" | tr -d "\n\t ")"

		# collect spim output
		mine="$(cat "$test.stdin" | spim -file "$codout" | sed '1d' | tr -d "\n\t ")"

		if [ "$orig" = "$mine" ]; then
				# I did have it echoing successes but it was too verbose
				# echo "PASS $test"
				true
			else
				echo "FAIL $test" 1>&2
		fi
	fi
done

rm "$codout"