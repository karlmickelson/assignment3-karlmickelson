# !/bin/bash
# Script to open qemu terminal.
# Author: Siddhant Jajoo.

set -e

OUTDIR=$1

if [ -z "${OUTDIR}" ]; then
    OUTDIR=/tmp/aeld
    echo "No outdir specified, using ${OUTDIR}"
fi

#cp -u /home/karl/assignment1/finder-app/versatile-pb.dtb ${OUTDIR}
echo "boot directory:"
ls -l ${OUTDIR}/linux-stable/arch/arm64/boot/
sudo cp ${OUTDIR}/linux-stable/arch/arm64/boot/Image ${OUTDIR}
KERNEL_IMAGE=${OUTDIR}/Image
#KERNEL_IMAGE=${OUTDIR}/linux-stable/arch/arm64/boot/Image
#KERNEL_IMAGE=${OUTDIR}/uRamdisk
INITRD_IMAGE=${OUTDIR}/initramfs.cpio.gz
#INITRD_IMAGE=${OUTDIR}/Image

if [ ! -e ${KERNEL_IMAGE} ]; then
    echo "Missing kernel image at ${KERNEL_IMAGE}"
    exit 1
fi
if [ ! -e ${INITRD_IMAGE} ]; then
    echo "Missing initrd image at ${INITRD_IMAGE}"
    exit 1
fi


echo "Booting the kernel"
# See trick at https://superuser.com/a/1412150 to route serial port output to file
qemu-system-aarch64 -m 256M -M virt -cpu cortex-a53 -nographic -smp 1 -kernel ${KERNEL_IMAGE} \
        -chardev stdio,id=char0,mux=on,logfile=${OUTDIR}/serial.log,signal=off \
        -serial chardev:char0 -mon chardev=char0\
        -append "rdinit=/bin/sh"  -initrd ${INITRD_IMAGE}
#qemu-system-aarch64 -m 256M -M virt -cpu cortex-a53 -nographic -kernel ${KERNEL_IMAGE} \
        #-append "console=ttyAMA0 rdinit=/bin/sh" -dtb ${OUTDIR}/virt.dtb
        #-append "rdinit=/bin/sh" -initrd ${INITRD_IMAGE}
#qemu-system-aarch64 -m 256M -nographic -M versatilepb -kernel ${KERNEL_IMAGE} \
        #-append "console=ttyAMA0 rdinit=/bin/sh" -initrd ${INITRD_IMAGE}
        #-append "console=ttyAMA0 rdinit=/bin/sh" -dtb ${OUTDIR}/versatile-pb.dtb -initrd ${INITRD_IMAGE}
        #-append "console=ttyAMA0 rdinit=/bin/sh" -initrd ${INITRD_IMAGE}
