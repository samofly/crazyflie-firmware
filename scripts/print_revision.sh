#!/usr/bin/env bash

REV=$(git rev-parse HEAD)
LOCAL=""
TAG=""

#LOCAL=$(hg identify -n)
#REV=$(hg identify -i)
#TAG=$(hg identify -t)

echo -n Build $REV \($TAG\) 

if git diff --quiet HEAD; then
  echo -e " \033[1;32mCLEAN\033[m"
else
  echo -e " \033[1;31mMODIFIED\033[m"
fi
