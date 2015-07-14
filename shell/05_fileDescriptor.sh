#! /bin/bash

touch out.txt
echo helloworld >> out.txt
exec 5>>out.txt
echo onemore helloworld >&5
echo "The content below is from out.txt"
cat out.txt -n
rm out.txt
