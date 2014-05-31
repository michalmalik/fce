#!/bin/bash

# === What?
# Restore a disk from an image. Only 2 partitions are currently supported.
# === Why?
# In case you screw something up.

IMAGES_DIR="/images"

function usage() {
cat << EOF

WARNING: Do not(!!) use it on mounted disks.
WARNING: Only 2 partitions are supported!

Disk will be restored from $IMAGES_DIR

Usage:	$0 <image_name> <disk_name>
	$0 list

Example:

Restore disk /dev/sdb and its partitions /dev/sdb1,dev/sdb2
from image $IMAGES_DIR/win7_VBoxUbuntu1204.

$0 win7_VBoxUbuntu1204 sdb

EOF
}

if [ `whoami` != "root" ]; then
	echo "root privileges required"
	exit 1
fi

if [ "$1" = "list" ]; then
	du -h $IMAGES_DIR | grep -v "${IMAGES_DIR}$" | sort
	exit 0
fi 

if [ -z "$1" ] || [ -z "$2" ]; then
	usage
	exit 1
fi

DEVICE_NAME="$2"
DEVICE_PATH="/dev/"$DEVICE_NAME
DISK1=$DEVICE_PATH"1"
DISK2=$DEVICE_PATH"2"

if [ ! -L "/sys/block/$DEVICE_NAME" ]; then
	echo "assuming $DEVICE_PATH does not exist"
	echo "/sys/block/$DEVICE_NAME does not exist or is not a symbolic link"
	exit 1
fi

if [ ! -d "/sys/block/$DEVICE_NAME/${DEVICE_NAME}1" ]; then
	echo "partition $DISK1 does not exist"
	exit 1
fi

if [ ! -d "/sys/block/$DEVICE_NAME/${DEVICE_NAME}2" ]; then
	echo "partition $DISK2 does not exist"
	exit 1
fi

mount | grep "$DEVICE_NAME" > /dev/null

if [ "$?" -eq 0 ]; then
	echo "I said DON'T use it on mounted disks."
	exit 1
fi

IMAGE_DIR=$IMAGES_DIR"/"$1
IMAGE_MBR_IMG=$IMAGE_DIR"/mbr.img"
IMAGE_MBR_SF=$IMAGE_DIR"/mbr.sf"
IMAGE_DISK1=$IMAGE_DIR"/part1.img"
IMAGE_DISK2=$IMAGE_DIR"/part2.img"

if [ ! -d $IMAGE_DIR ]; then
	echo "image \"$1\" does not exist"
	exit 1
fi

if [ ! -f $IMAGE_MBR_IMG ]; then
	echo $IMAGE_MBR_IMG" does not exist"
	exit 1
fi

if [ ! -f $IMAGE_MBR_SF ]; then
	echo $IMAGE_MBR_SF" does not exist"
	exit 1
fi

if [ ! -f $IMAGE_DISK1 ]; then
	echo $IMAGE_DISK1" does not exist"
	exit 1
fi

if [ ! -f $IMAGE_DISK2 ]; then
	echo $IMAGE_DISK2" does not exist"
	exit 1
fi

DISK1_BYTE_SIZE=$(du -h -b "$IMAGE_DISK1" | awk '{print $1}')
DISK1_SIZE=$(($DISK1_BYTE_SIZE/(1024*1024)))

DISK2_BYTE_SIZE=$(du -h -b "$IMAGE_DISK2" | awk '{print $1}')
DISK2_SIZE=$(($DISK2_BYTE_SIZE/(1024*1024)))

echo ""
echo "restoring MBR from $IMAGE_MBR_SF"
echo "-------------------------------------------"
echo ""

sfdisk $DEVICE_PATH < $IMAGE_MBR_SF --force

echo ""

echo "restoring $DISK1 ("$DISK1_SIZE" MB)"
echo "-------------------------------------------"
echo ""

dd if=$IMAGE_DISK1 2> /dev/null | pv -tpreb -s $DISK1_SIZE"M" | dd of=$DISK1 bs=100M 2> /dev/null

echo ""

echo "restoring $DISK2 ("$DISK2_SIZE" MB)"
echo "-------------------------------------------"
echo ""

dd if=$IMAGE_DISK2 2> /dev/null | pv -tpreb -s $DISK2_SIZE"M" | dd of=$DISK2 bs=100M 2> /dev/null

echo ""

echo "-------------------------------------------"
echo ""
echo "image restored ok"
echo ""
