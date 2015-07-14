#! /bin/bash
 echo helloworld | tee out.txt | cat -n

 # You can see 1 helloworld, which is output by "cat -n"
 # More important, tee redirect stdout to out.txt and dup it to the stdin
 # which was accepted by cat
 # - can be seen as stdin
