#!/bin/bash
if [ $# -gt 0 ]; then
IMGNAME=$1
else
IMGNAME=AW
fi

cd $PROJECT_BUILD_DIR

ENTRY=`readelf -a $PROJECT_TOP_DIR/build/$PRODUCT_SERIES/vmlinux|grep "Entry" |cut -d":" -f 2`
LDADDR=`readelf -a $PROJECT_TOP_DIR/build/$PRODUCT_SERIES/vmlinux|grep "\[ 1\]"|cut -d" " -f 26`

compress_bin()
{
echo "Convert the kernel into a raw binary image ..."
arm-marvell-linux-gnueabi-objcopy -O binary -R .note -R .comment -S vmlinux linux.bin
echo "Compressing the kernel bin..."
gzip -9 -f linux.bin
echo "Packaging compressed image for U-boot"
$PROJECT_TOP_DIR/mkimage-ls -A arm -O linux -T kernel -C gzip -a 0x00008000 -e 0x00008000 -n $IMGNAME -d linux.bin.gz AW.IMG
}

compress_elf()
{
echo "Compressing the kernel elf..."
gzip -9 -f vmlinux
echo "Packaging compressed image for U-boot"
$PROJECT_TOP_DIR/mkimage-ls -A arm -O linux -T kernel -C gzip -a 0x00008000 -e 0x00008000 -n $IMGNAME -d vmlinux.gz AW.IMG
}

# uncompress and load bin is not supported by boot by now
#compress_elf
compress_bin

echo "Verifying image information ... "
$PROJECT_TOP_DIR/mkimage-ls -l AW.IMG
cd ..
