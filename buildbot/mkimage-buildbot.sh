#!/bin/bash

set -ue

if [ $# -ne 3 ]
then
    echo "Usage: ./mkimage-buildbot.sh <image_name> <password> <web port>"
    exit 1
fi

export IMAGE_NAME=$1
export PASSWORD=$2
export WEB_PORT=$3

docker rmi $IMAGE_NAME || echo "Old buildbot image not found, so nothing to delete"

readonly TMP_DIR=`mktemp -d`
readonly DOCKERFILE="$TMP_DIR/Dockerfile"

envsubst < Dockerfile.buildbot > $DOCKERFILE

cd $TMP_DIR
docker build -t $IMAGE_NAME .
echo Dockefile: $DOCKERFILE
