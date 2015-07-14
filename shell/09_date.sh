#! /bin/bash

start=$(date +%s) # inside the brace... run a command
read TMP
echo $TMP
end=$(date +%s)
echo $[ end - start ]
