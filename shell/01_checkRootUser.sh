#! /bin/bash

if [ $UID -ne 0 ];then # Note here the space is quite important
  echo Non root user
else
  echo Root user
fi
