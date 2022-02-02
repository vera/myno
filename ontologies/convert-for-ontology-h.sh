#!/bin/bash
if [ "$#" -ne 1 ]; then
    echo "Usage: ./convert-for-ontology-h.sh PATH-TO-ONTOLOGY"
    echo "Removes all extra spaces, newlines and escapes quotation marks."
    exit 1
fi

# Source for awk command: https://stackoverflow.com/a/17302816
cat $1 | awk '
    BEGIN {FS = OFS = "\""}
    /^[[:blank:]]*$/ {next}
    {for (i=1; i<=NF; i+=2) gsub(/[[:space:]]/,"",$i)} 
    1
' | tr -d '\n' | sed 's/"/\\"/g'
echo ""