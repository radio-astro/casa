#! /bin/bash

# Tack dummy values in svninfo.txt
OUT=python/svninfo.txt
SRCVER="1"
HTTPS="git"
echo "$HTTPS" > $OUT
echo "$SRCVER" >> $OUT
date "+%b %d %Y, %H:%M:%S" >> $OUT

