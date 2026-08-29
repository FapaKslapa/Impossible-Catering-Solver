#!/bin/sh
set -e
BINARY=$1
shift
STATUS=0
for input in "$@"; do
    expected="${input%.txt}.output.txt"
    if [ ! -f "$expected" ]; then
        echo "SKIP (no expected output): $input"
        continue
    fi
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
