#!/bin/bash

# temporary file for parser output
semout="$(mktemp)"

# names of test scripts stripped of extentions
tests="$(find ./correct_sem/ | sed -E 's/\.in|\.out//g' | uniq)"

for test in $tests; do

	# dump ast to temporary file
	./semantics "$test.in" "$semout" 2> /dev/null

	if [ "$?" = 2 ]; then
		echo "SEGV $test"
	else
		# strip the whitespace from both
		mine="$(cat "$semout" | tr -d "\n\t ")"

		# compare them
		if [ ! -e "$test.out" ]; then

			if [ ! -z "$mine" ]; then

				echo "FAIL $test"

			fi

		else

			orig="$(cat "$test.out"  | tr -d "\n\t ")"

			if [ "$orig" = "$mine" ]; then
				# I did have it echoing successes but it was too verbose
				# echo "PASS $test"
				true
			else
				echo "FAIL $test"
			fi

		fi
		
	fi
	
done

rm "$semout"