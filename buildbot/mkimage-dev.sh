#!/bin/bash

set -ue

if [ $# -ne 4 ]
then
    echo "Usage: ./mkimage-dev.sh <base_image> <image_name> <public_key_file> <port_range_start>"
    exit 1
fi

export BASE_IMAGE=$1
export IMAGE_NAME=$2
export PUBLIC_KEY_FILE=$3
export PORT_RANGE_START=$4

export PORT0=$PORT_RANGE_START
export PORT1=$((PORT_RANGE_START+1))
export PORT2=$((PORT_RANGE_START+2))
export PORT3=$((PORT_RANGE_START+3))
export PORT4=$((PORT_RANGE_START+4))
export PORT5=$((PORT_RANGE_START+5))

docker rmi $IMAGE_NAME || echo "Old image not found, so nothing to delete"

readonly TMP_DIR=`mktemp -d`
readonly DOCKERFILE="$TMP_DIR/Dockerfile"
readonly SSH_ID="$TMP_DIR/ssh_id"

envsubst < Dockerfile.dev > $DOCKERFILE
cp $PUBLIC_KEY_FILE $SSH_ID
cp sudoers.dev $TMP_DIR/sudoers

cd $TMP_DIR
docker build -t $IMAGE_NAME .
echo Dockefile: $DOCKERFILE
