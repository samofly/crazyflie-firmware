#!/bin/bash

set -ue

if [ $# -ne 4 ]
then
    echo "Usage: ./mkimage-buildbot.sh <base_image> <image_name> <password> <web port>"
    exit 1
fi

export BASE_IMAGE=$1
export IMAGE_NAME=$2
export PASSWORD=$3
export WEB_PORT=$4

docker rmi $IMAGE_NAME || echo "Old buildbot image not found, so nothing to delete"

readonly TMP_DIR=`mktemp -d`
readonly DOCKERFILE="$TMP_DIR/Dockerfile"
readonly MASTER_CFG="$TMP_DIR/master.cfg"

envsubst < Dockerfile.buildbot > $DOCKERFILE
envsubst < master.cfg > $MASTER_CFG

cd $TMP_DIR
docker build -t $IMAGE_NAME .
echo Dockefile: $DOCKERFILE
