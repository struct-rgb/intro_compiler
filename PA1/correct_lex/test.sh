#!/bin/sh

./scanner "$1.in" "$1.txt"
diff "$1.out" "$1.txt"

