#!/bin/bash
if [ $# -gt 0 ]; then
IMGNAME=$1
else
IMGNAME=AW
fi

cd $PROJECT_BUILD_DIR

compress_bin()
{
echo "Convert the kernel into a raw binary image ..."
mips64-octeon-linux-gnu-objcopy -O binary -R .note -R .comment -S vmlinux linux.bin
echo "Compressing the kernel bin..."
gzip -9 -f linux.bin
echo "Packaging compressed image for U-boot"
$PROJECT_TOP_DIR/mkimage -A mips -O linux -T kernel -C gzip -a 9000000 -n $IMGNAME -d linux.bin.gz AW.IMG
}

compress_elf()
{
echo "Compressing the kernel elf..."
gzip -9 -f vmlinux
echo "Packaging compressed image for U-boot"
$PROJECT_TOP_DIR/mkimage -A mips -O linux -T kernel -C gzip -a 9000000 -n $IMGNAME -d vmlinux.gz AW.IMG
}

# uncompress and load bin is not supported by boot by now
compress_elf

echo "Verifying image information ... "
$PROJECT_TOP_DIR/mkimage -l AW.IMG
cd ..
