#!/bin/bash

if [ "$#" -ne 1 ]; then
    echo "Eroare"
    exit 1
fi

c="$1"
corect=0

while IFS= read -r linie; do
    if [[ $linie =~ ^[A-Z][A-Za-z0-9\ \,]*[\?\!\.]$ && ! $line =~ ,\ (si) ]]; then
        if [[ $linie == *"$c"* ]]; then
            ((corect++))
        fi
    fi
done

echo "$corect"
