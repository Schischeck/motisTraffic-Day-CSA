#!/bin/bash

url="http://localhost:8080/routing"
filename="queries.txt"

if [ $# -gt 1 ] || [ $# -eq 1 ]
 then
  filename="$1"
fi

if [ $# -eq 2 ]
 then
  url="$2"
fi

curl -s --data @"$filename" --url "$url" \
    | tidy -w 120 -i -utf8 -xml -q \
    | less -R
