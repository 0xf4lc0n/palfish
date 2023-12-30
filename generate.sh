#!/bin/sh

if [[ $1 = "key" ]] && [ ! -z $2 ];  then
    size=$2
elif [[ $1 = "file" ]] && [ ! -z $2 ]; then
    size=$(echo "$2 * 1048576" | bc)
else
    echo "Usage: ./generate.sh [key|file] [size]"
    echo "Size is in bytes for key and in MB for file"
    exit
fi

echo -n "$(tr -dc '[:alnum:]' </dev/urandom | head -c $size)"