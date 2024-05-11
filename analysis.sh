#!/bin/bash

# Check if a file is provided as argument
if [ $# -eq 0 ]; then
    echo "Usage: $0 <filename>"
    exit 1
fi

# Check if the file exists
if [ ! -f "$1" ]; then
    echo "Error: File '$1' not found."
    exit 1
fi

# Count the number of lines, words, and characters in the file
num_lines=$(wc -l < "$1")
num_words=$(wc -w < "$1")
num_chars=$(wc -m < "$1")

# Check the conditions
if [ "$num_lines" -lt 3 ]; then
    if [ "$num_words" -gt 1000 ]; then
        if [ "$num_chars" -gt 2000 ]; then
            "Potentially malicious file ->" "$1"
            exit 1;
        fi
    fi
fi
if file "$1" | grep -q "non-ASCII"; then
    echo "Potentially malicious file ->" "$1"
    exit 1
fi
if grep -qEi 'corrupted|dangerous|risk|attack|malware|malicious' "$1"; then
    echo "Potentially malicious file ->" "$1"
    exit 1
fi
echo "False positive ->" "$1"
exit 0
