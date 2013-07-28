#!/bin/bash

set -ue

if [ $# -ne 1 ]
then
    echo "Usage: ./mkimage-buildbot-base.sh <image_name>"
    exit 1
fi

export IMAGE_NAME=$1

docker rmi $IMAGE_NAME || echo "Old buildbot image not found, so nothing to delete"

readonly TMP_DIR=`mktemp -d`
readonly DOCKERFILE="$TMP_DIR/Dockerfile"

cp Dockerfile.buildbot-base $DOCKERFILE

cd $TMP_DIR
docker build -t $IMAGE_NAME .
echo Dockefile: $DOCKERFILE
