#!/bin/sh
set -e
BINARY=$1
shift
STATUS=0
for input in "$@"; do
    expected="${input%.txt}.output.txt"
    actual=$("$BINARY" "$input")
    expected_content=$(cat "$expected")
    if [ "$actual" = "$expected_content" ]; then
        echo "PASS: $input"
    else
        echo "FAIL: $input"
        echo "  expected: $expected_content"
        echo "  actual:   $actual"
        STATUS=1
    fi
done
exit $STATUS
