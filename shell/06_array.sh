#! /bin/bash

# array
arr=(1 2 3 4 5 6)
echo The 3th is ':' ${arr[2]}
echo All element ':' ${arr[*]}
echo Length ':' ${#arr[*]}

# map
declare -A ass_array

ass_array[apple]=100
ass_array[pear]=200

echo ${ass_array[apple]}
echo All the keys ':' ${!ass_array[*]}
