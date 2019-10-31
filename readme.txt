TP-LINK GPL **U-Boot** code readme

1. This package contains **only** U-Boot GPL code used by TP-Link TL-WR841ND v9 Router
and TP-Link's ancient 32-bit uClibc gcc-4.3.3 toolchain currently needed to build it.
Changes are required for building with another toolchain, see mikebdp2's new repository:
https://github.com/mikebdp2/wr841nd_v9_u-boot_r3b0rn

2. U-Boot build OK on Artix Linux ( https://artixlinux.org/ great fresh no-SystemD Arch )
with multilib ( https://wiki.archlinux.org/index.php/Official_repositories#multilib ) and
32-bit toolchain needs some 32-bit packages: lib32-glibc, lib32-zlib and lib32-gcc-libs.

3. All the other stuff from this package has been removed for simplicity: if you need it,
please get a complete package from https://github.com/mikebdp2/wr841nv9_en_gpl

tar -zxvf ./wr841nv9_en_gpl.tar.gz
cd ./wr841nv9_en_gpl/
rm -rf ./ap143/apps/ && rm -rf ./ap143/linux/ && rm -rf ./apps/
rm -rf ./filesystem/ && rm -rf ./kernel_modules/ && rm -rf ./pb92/
rm -rf ./toolchain_src/ && rm -rf ./util/ && rm -rf ./web_server/
mkdir ./util/ && mkdir ./util/lzma/ && mkdir ./util/lzma/bin/
which lzma
# /usr/bin/lzma
ln -s /usr/bin/lzma ./util/lzma/bin/lzma
nano ./readme.txt

BOARD_TYPE definitions
1. ap143: TL-WR841N/ND 9.0

DEV_NAME definitions
1. wr841nv9_en: TL-WR841N/ND 9.0

Build Instructions
1. All build targets are in ./wr841nv9_en_gpl/build/Makefile ,
you should enter this directory to build components:

        cd ./path_to/wr841nv9_en_gpl/build/Makefile

2. Pre-built 32-bit uClibc gcc-4.3.3 toolchain is available in this package,
however its' source code is available only at toolchain_src of complete package.
Prepare this toolchain:

        make DEV_NAME=wr841nv9_en toolchain_prep

3. Build u-boot bootloader:

        make DEV_NAME=wr841nv9_en uboot

Required U-Boot file will be called " tuboot.bin " and located at

        ls -al ./../ap143/boot/u-boot/tuboot.bin

4. Generate a 128 KB file of 0xff :

        tr '\0' '\377' < /dev/zero | dd bs=1024 count=128 of=./128kb.bin

Then overwrite its' beginning with your " tuboot.bin " :

        dd if=./../ap143/boot/u-boot/tuboot.bin of=./128kb.bin conv=notrunc

Now, using macmodelpin from https://github.com/mikebdp2/macmodelpin , print a macmodelpin
of your router's full chip dump and set the same or changed values near the end of 128KB:

        git clone https://github.com/mikebdp2/macmodelpin
        cd ./macmodelpin/
        gcc -o macmodelpin macmodelpin.c
        ./macmodelpin ./path_to/dump.bin
        ./macmodelpin ./../128kb.bin 0xMAC 0xMODEL PIN
        ./macmodelpin ./../128kb.bin 0x16A2594B37DF 0x084100090000001 12345678

5. Overwrite the first 128kb of your router's full chip dump, before flashing to a chip:

        dd if=./../128kb.bin of=./path_to/dump.bin conv=notrunc
        sudo ./path_to/flashrom -p ch341a_spi -c "MX25L3206E/MX25L3208E" -w ./dump.bin -V

Happy Hacking!
