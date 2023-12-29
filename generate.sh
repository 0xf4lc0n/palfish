#!/bin/sh

if [[ $1 = "key" ]] && [ ! -z $2 ];  then
    size=$2
elif [[ $1 = "file" ]] && [ ! -z $2 ]; then
    size=$(echo "$2 * 1048576" | bc)
else
    echo "Usage: ./generate.sh [key|file] [size]" && exit
fi

tr -dc '[:alnum:]' </dev/urandom | head -c $size > "${1}.txt"
echo "Created ${1}.txt with size $size"