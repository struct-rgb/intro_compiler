#!/bin/bash

# temporary file for parser output
parserout="$(mktemp)"

# names of test scripts stripped of extentions
tests="$(find ./correct_syn/ | sed -E 's/\.in|\.out//g' | uniq)"

for test in $tests; do

	# dump ast to temporary file
	./parser "$test.in" "$parserout" 2> /dev/null

	if [ "$?" = 2 ]; then
		echo "SEGV $test"
	else
		# strip the whitespace from both
		orig="$(cat "$test.out"  | tr -d "\n\t ")"
		mine="$(cat "$parserout" | tr -d "\n\t ")"

		# compare them
		if [ "$orig" = "$mine" ]; then
			# I did have it echoing successes but it was too verbose
			# echo "PASS $test"
			true
		else
			echo "FAIL $test"
		fi
	fi

	
done

rm "$parserout"