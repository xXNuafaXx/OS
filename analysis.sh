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

echo "Number of lines: $num_lines"
echo "Number of words: $num_words"
echo "Number of characters: $num_chars"

# Search for keywords associated with corrupted or malicious files
echo "Searching for keywords associated with corrupted or malicious files..."

if grep -qEi 'corrupted|dangerous|risk|attack|malware|malicious|[^ -~]' "$1"; then
    echo "Potential corrupted or malicious content found."
    exit 1
else
    echo "No corrupted or malicious content found."
    exit 0
fi
