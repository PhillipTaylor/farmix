#!/usr/bin/env bash

# This script tars the working directory sends it to a remote machine
# and builds it

TARGET="localhost"
TARGET_PORT="2222"
TARGET_USERNAME="root"

TARGET_BUILD_DIR="/tmp"
TRANSFER_FILENAME="farmix.tar.gz"

LOCAL_BUILD_DIR="/tmp"

function transfer_to_target {

	tar -cf $LOCAL_BUILD_DIR/$TRANSFER_FILENAME ../farmix
	scp -P $TARGET_PORT $LOCAL_BUILD_DIR/$TRANSFER_FILENAME $TARGET_USERNAME@$TARGET:$TARGET_BUILD_DIR
	scp -P $TARGET_PORT $0 $TARGET_USERNAME@$TARGET:$TARGET_BUILD_DIR
	ssh -p $TARGET_PORT -l $TARGET_USERNAME $TARGET -x $TARGET_BUILD_DIR/$0 continue

}

function continue_on_target {

	cd $TARGET_BUILD_DIR
	tar -xf $TRANSFER_FILENAME
	cd farmix
	make
}

if [ "$1" = "continue" ]
then
	echo "$(hostname): running continue_to_target"
	continue_on_target
else
	echo "$(hostname): running transfer_to_target"
	transfer_to_target
fi;

echo "$(hostname): script ended on"
