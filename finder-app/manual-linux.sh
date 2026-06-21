#!/bin/bash
# Script outline to install and build kernel.
# Author: Siddhant Jajoo.

set -e
set -u

OUTDIR=/tmp/aeld
KERNEL_REPO=git://git.kernel.org/pub/scm/linux/kernel/git/stable/linux-stable.git 
#from assignment instructions: https://git.kernel.org/pub/scm/linux/kernel/git/stable/linux.git
KERNEL_VERSION=v5.15.163
BUSYBOX_VERSION=1_33_0
FINDER_APP_DIR=$(realpath $(dirname $0))
ARCH=arm64
CROSS_COMPILE=aarch64-linux-gnu-

if [ $# -lt 1 ]
then
	echo "Using default directory ${OUTDIR} for output"
else
	OUTDIR=$1
	echo "Using passed directory ${OUTDIR} for output"
fi

mkdir -p ${OUTDIR}

cd "$OUTDIR"
if [ ! -d "${OUTDIR}/linux-stable" ]; then
    #Clone only if the repository does not exist.
	echo "CLONING GIT LINUX STABLE VERSION ${KERNEL_VERSION} IN ${OUTDIR}"
	git clone ${KERNEL_REPO} --depth 1 --single-branch --branch ${KERNEL_VERSION}
fi
if [ ! -e ${OUTDIR}/linux-stable/arch/${ARCH}/boot/Image ]; then
    cd linux-stable
    echo "Checking out version ${KERNEL_VERSION}"
    git checkout ${KERNEL_VERSION}

    # TODO: Add your kernel build steps here
    make ARCH=arm64 CROSS_COMPILE=aarch64-linux-gnu- mrproper
    make ARCH=arm64 CROSS_COMPILE=aarch64-linux-gnu- defconfig
    make -j4 ARCH=arm64 CROSS_COMPILE=aarch64-linux-gnu- all
    make ARCH=arm64 CROSS_COMPILE=aarch64-linux-gnu- dtbs
        
fi

echo "Adding the Image in outdir"

echo "Creating the staging directory for the root filesystem"
cd "$OUTDIR"
if [ -d "${OUTDIR}/rootfs" ]
then
    echo "Deleting rootfs directory at ${OUTDIR}/rootfs and starting over"
    sudo rm  -rf ${OUTDIR}/rootfs
fi

mkdir -p ${OUTDIR}/rootfs
cd ${OUTDIR}/rootfs
# TODO: Create necessary base directories
mkdir -p bin dev etc home lib lib64 proc sbin sys tmp usr var
mkdir -p usr/bin usr/lib usr/sbin
mkdir -p var/log
pwd
ls 

echo "karl, we're loading busybox"

cd "$OUTDIR"
if [ ! -d "${OUTDIR}/busybox" ]
then
git clone git://busybox.net/busybox.git
    echo "karl, we're cloning busybox"
    cd busybox
    git checkout ${BUSYBOX_VERSION}
    # TODO:  Configure busybox
    make distclean
    make defconfig
    
else
    cd busybox
    echo "karl, we're just going to cd over to busybox"
fi

echo "karl, we're making and installing busybox"

# TODO: Make and install busybox
make ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE}
make CONFIG_PREFIX=${OUTDIR}/rootfs ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE} install

echo "Library dependencies"
aarch64-linux-gnu-readelf -a ${OUTDIR}/rootfs/bin/busybox | grep "program interpreter"
aarch64-linux-gnu-readelf -a ${OUTDIR}/rootfs/bin/busybox | grep "Shared library"
aarch64-linux-gnu-readelf -a ${OUTDIR}/rootfs/bin/busybox | grep "NEEDED"

echo "karl, we'll add the dependencies"
# TODO: Add library dependencies to rootfs
# see 10:46 of "Linux Root Filesystems" video
mkdir -p ${OUTDIR}/rootfs/lib64
cp /usr/aarch64-linux-gnu/lib/ld-linux-aarch64.so.1 ${OUTDIR}/rootfs/lib
cp /usr/aarch64-linux-gnu/lib/libm.so.6 ${OUTDIR}/rootfs/lib
cp /usr/aarch64-linux-gnu/lib/libresolv.so.2 ${OUTDIR}/rootfs/lib
cp /usr/aarch64-linux-gnu/lib/libc.so.6 ${OUTDIR}/rootfs/lib
ls

# TODO: Make device nodes
cd ${OUTDIR}/rootfs
mkdir -p dev
#cd dev
sudo mknod -m 666 dev/null c 1 3
sudo mknod -m 666 dev/console c 5 1
ls -l dev

echo "karl, build writer"
# TODO: Clean and build the writer utility
cd /home/karl/assignment1/finder-app
ls
make all writer

echo "karl, copy finder stuff over to rootfs"
# TODO: Copy the finder related scripts and executables to the /home directory
# on the target rootfs
mkdir -p $OUTDIR/rootfs/conf
cd "$OUTDIR/rootfs"
cp -u /home/karl/assignment1/finder-app/finder.sh .
cp -u /home/karl/assignment1/finder-app/finder-test.sh .
cp -u /home/karl/assignment1/finder-app/conf/assignment.txt $OUTDIR/rootfs/conf
cp -u /home/karl/assignment1/finder-app/conf/username.txt $OUTDIR/rootfs/conf
cp -u /home/karl/assignment1/finder-app/autorun-qemu.sh $OUTDIR/rootfs
cp -u /home/karl/assignment1/finder-app/writer.o .
cp -u /home/karl/assignment1/full-test.sh ./
cp -u /home/karl/assignment1/unit-test.sh .
cp -u -r /home/karl/assignment1/examples ./examples
cp -u -r /home/karl/assignment1/finder-app ./finder-app
cp -u -r /home/karl/assignment1/finder-app/conf ./finder-app/conf
cp -u -r /home/karl/assignment1/build ./build


echo "karl, do chown"
# TODO: Chown the root directory
cd $OUTDIR/
sudo chown -R root:root *
sudo chown root:root /bin/busybox
sudo chown root:root ${OUTDIR}/rootfs/bin/busybox

echo "karl, do initramfs"
# TODO: Create initramfs.cpio.gz
cd "$OUTDIR/rootfs"
echo "directory of rootfs:"
ls
find . | cpio -H newc -ov --owner root:root > ../initramfs.cpio
#find . | cpio --null -ov --format=newc | gzip -9 > ../initramfs.cpio.gz
cd ..
gzip -f initramfs.cpio
echo "karl, gzip is done"
#mkimage -A arm64 -O linux -T ramdisk -d initramfs.cpio.gz uRamdisk
#mkimage -A arm64 -O linux -T kernel -d initramfs.cpio.gz zImage
#mkimage -A arm64 -O linux -T kernel -d initramfs.cpio.gz Image


