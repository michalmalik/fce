#!/bin/bash

# === What?
# Make an image of a disk. Currently supports only 2 partitions.
# Just like virtual machine snapshots.
# === Why?
# It makes restoring and switching between OSes far easier than
# installing them all over again. 

IMAGES_DIR="/images"

function usage() {
cat << EOF

WARNING: Do not(!!) use it on mounted disks.
WARNING: Only 2 partitions are supported!

Image will be stored into $IMAGES_DIR

Usage:	$0 <image_name> <disk_name>

Example:

Make image of the disk /dev/sdb and its partitions /dev/sdb1,/dev/sdb2.
Store it into $IMAGES_DIR/win7_VBoxUbuntu1204.

$0 win7_VBoxUbuntu1204 sdb

EOF
}

if [ `whoami` != "root" ]; then
	echo "root privileges required"
	exit 1
fi

if [ -z "$1" ] || [ -z "$2" ]; then
	usage
	exit 1
fi

DEVICE_NAME="$2"
DEVICE_PATH="/dev/"$DEVICE_NAME
DISK1=$DEVICE_PATH"1"
DISK2=$DEVICE_PATH"2"

mount | grep "$DEVICE_NAME" > /dev/null

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

if [ "$?" -eq 0 ]; then
	echo "I said DON'T use it on mounted disks."
	exit 1
fi

DISK1_SYS_SIZE=$(cat /sys/block/$DEVICE_NAME/$DEVICE_NAME"1"/size)
DISK1_BYTE_SIZE=$(($DISK1_SYS_SIZE*512))
DISK1_SIZE=$(($DISK1_BYTE_SIZE/(1024*1024)))

DISK2_SYS_SIZE=$(cat /sys/block/$DEVICE_NAME/$DEVICE_NAME"2"/size)
DISK2_BYTE_SIZE=$(($DISK2_SYS_SIZE*512))
DISK2_SIZE=$(($DISK2_BYTE_SIZE/(1024*1024)))

IMAGE_DIR=$IMAGES_DIR"/"$1
IMAGE_MBR_IMG=$IMAGE_DIR"/mbr.img"
IMAGE_MBR_SF=$IMAGE_DIR"/mbr.sf"
IMAGE_DISK1=$IMAGE_DIR"/part1.img"
IMAGE_DISK2=$IMAGE_DIR"/part2.img"

if [ -d $IMAGE_DIR ]; then
	echo "image \"$1\" probably already exists"
	exit 1
fi

mkdir $IMAGE_DIR

echo ""

echo "backing up MBR"
echo "-------------------------------------------"
echo ""

dd if=$DEVICE_PATH 2> /dev/null of=$IMAGE_MBR_IMG bs=512 count=1 2> /dev/null
sfdisk -d $DEVICE_PATH > $IMAGE_MBR_SF 2> /dev/null

echo ""

echo "backing up $DISK1 ("$DISK1_SIZE" MB)"
echo "-------------------------------------------"
echo ""

dd if=$DISK1 2> /dev/null | pv -tpreb -s $DISK1_SIZE"M" | dd of=$IMAGE_DISK1 2> /dev/null

echo ""

echo "backing up $DISK2 ("$DISK2_SIZE" MB)"
echo "-------------------------------------------"
echo ""

dd if=$DISK2 2> /dev/null | pv -tpreb -s $DISK2_SIZE"M" | dd of=$IMAGE_DISK2 2> /dev/null

echo ""

echo "-------------------------------------------"
echo ""
echo "image made ok"
echo ""

