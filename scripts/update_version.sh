#!/usr/bin/env bash

FILE=utils/src/version.c

#Get the relevant informations

REV=$(git rev-parse HEAD)
LOCAL=""
TAG=""

#LOCAL=$(hg identify -n)
#REV=$(hg identify -i)
#TAG=$(hg identify -t)
#BRANCH=$(hg identify -b)

if git diff --quiet HEAD; then
  MODIFIED=0
else
  MODIFIED=1
fi

#Patch version.c
sed -i "s/SLOCAL_REVISION=\".*\";\$/SLOCAL_REVISION=\"$LOCAL\";/" $FILE
sed -i "s/STAG=\".*\";\$/STAG=\"$TAG\";/" $FILE
sed -i "s/SREVISION=\".*\";\$/SREVISION=\"$REV\";/" $FILE
sed -i "s/MODIFIED=.*;\$/MODIFIED=$MODIFIED;/" $FILE

true
