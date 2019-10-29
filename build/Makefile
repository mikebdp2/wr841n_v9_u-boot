ifndef DEV_NAME
$(error "You have to define Dev name to use this Makefile")
endif

export TOPDIR=$(PWD)/..
export INSTALL_ROOT=$(TOPDIR)/rootfs.build.$(KERNELVER)
export INSTALL_ROOT_FOR_BOARD=$(TOPDIR)/rootfs.$(DEV_NAME)
export IMAGE_ROOT=$(TOPDIR)/rootfs-$(BOARD_TYPE).optbuild
temp_BOARD_TYPE = $(strip $(subst fus, , $(BOARD_TYPE)))

export PRODUCT_CONFIG_DIR=$(TOPDIR)/build/products_config/$(DEV_NAME)/
# modules used by all boards
export BOARD_KERNEL_MODULES = netfilter netsched ts_kmp pppol2tp pptp_module
# applications used by all board types 
export BOARD_APPLICATIONS = busybox pppoe l2tp bpa iptables tc_build lltd arp

include $(PRODUCT_CONFIG_DIR)/basic.config

ifndef BOARD_TYPE
export BOARD_TYPE=ap143
endif

ifndef BUILD_TYPE
export BUILD_TYPE=squashfs
endif

ifndef TOOLCHAIN
$(error "Must specify TOOLCHAIN value")
endif

ifndef TOOLARCH
$(error "Must specify TOOLARCH value")
endif

ifndef KERNEL
$(error "Must specify KERNEL value")
endif

ifndef KERNELVER
$(error "Must specify KERNELVER value")
endif

ifndef KERNELTARGET
$(error "Must specify KERNELTARGET value")
endif

ifndef KERNELARCH
$(error "Must specify KERNELARCH value")
endif

ifndef TFTPPATH
export TFTPPATH=$(TOPDIR)/tftpboot/$(DEV_NAME)/
endif

# To enable library optimization (removing unused object code from
# shared libraries) set BUILD_LIBRARYOPT to 'y' else 'n'
BUILD_LIBRARYOPT := n

ifndef  BUILD_OPTIMIZED
export BUILD_OPTIMIZED=n
export STRIP=$(TOOLPREFIX)strip
endif

ifeq ($(BUILD_OPTIMIZED), y)
export OLDSTRIP=$(TOOLPREFIX)strip
export STRIP=$(TOOLPREFIX)sstrip
endif

ifndef COMPRESSED_UBOOT
export COMPRESSED_UBOOT=0
endif

ifeq ($(BOOT_FROM_NAND),1)
export NAND=-nand
else
export NAND=
endif

#export IMAGEPATH=$(TOPDIR)/images/$(BOARD_TYPE)$(BUILD_CONFIG)$(BUILD_EXT)$(NAND)
export IMAGEPATH=$(TOPDIR)/images/$(DEV_NAME)

export INFOFILE=vmlinux.info
export KTFILE=$(KERNELTARGET)

ifneq ($(COMPRESSED_UBOOT),1)
export UBOOTFILE=u-boot.bin
export UBOOT_BINARY=u-boot.bin
else
export UBOOTFILE=tuboot.bin
export UBOOT_BINARY=tuboot.bin
endif
export UBOOT_FILE=u-boot.bin

#
# Other environmental variables that are configured as per the configuration file
# specified above.  These contain all platform specific configuration items.
#

ifeq ($(strip $(ATH_CARR_DIR)),)
export KERNELPATH=$(TOPDIR)/$(BOARD_TYPE)/linux/kernels/$(KERNEL)
export MAKEARCH=$(MAKE) ARCH=$(KERNELARCH) CROSS_COMPILE=$(TOOLPREFIX)

export TOOLPATH=$(TOPDIR)/build/$(TOOLCHAIN)/$(TOOLARCH)/
export BOOTLOADERDIR=$(TOPDIR)/$(BOARD_TYPE)/boot/u-boot

export UBOOTDIR=$(TOPDIR)/$(BOARD_TYPE)/boot/u-boot
endif

# Save PATH for later use for compiling host-only tools etc.
#~export ORIGINAL_PATH:=$(PATH)
# Set PATH so we find target compiler when say "gcc", etc.
# as well as other tools we expect to find.
export PATH:=$(TOOLPATH)/bin:${PATH}

# madwifi
export MODULEPATH=$(INSTALL_ROOT)/lib/modules/$(KERNELVER)/net

KERNEL_MODULE_PATH=$(TOPDIR)/images/$(DEV_NAME)/kernel_modules/
LIBMODULESDIR_KERNEL=$(INSTALL_ROOT_FOR_BOARD)/lib/modules/$(KERNELVER)/kernel
KERNEL_MODULES_IMAGE=$(IMAGEPATH)/kernel_modules

######## build fakeroot host tool ########
fakeroot_build:
	cd $(TOPDIR)/util/fakeroot/fakeroot-1.18.2 && \
	if test -e Makefile; then make distclean; fi ;
	cd $(TOPDIR)/util/fakeroot/fakeroot-1.18.2 && ./configure --prefix=$(TOPDIR)/util/fakeroot/install
	cd $(TOPDIR)/util/fakeroot/fakeroot-1.18.2 && make
	cd $(TOPDIR)/util/fakeroot/fakeroot-1.18.2 && make install
	
######## prepare toolchain ########
toolchain_prep:
	@echo Uncompressing toolchain, please wait for a few minutes ...
	cd $(TOPDIR)/toolchain && \
	tar -jxf gcc-4.3.3.tar.bz2 && \
	mv gcc-4.3.3 $(TOPDIR)/build


#
# common build
#
fs_prep:install_root_clean util_chmod check_tftp libs_copy etc_prep empty_dir_prep

install_root_clean:
	-rm -rf $(INSTALL_ROOT)/*
	
util_chmod:
	-cd $(TOPDIR)/util && chmod 777 cplibs mksquashfs4-ap143 
	
check_tftp:
	mkdir -p $(IMAGEPATH)
	if test -d $(TFTPPATH); then echo $(TFTPPATH) exists; else mkdir -p $(TFTPPATH); fi;

libs_copy:
	$(TOPDIR)/util/cplibs
	
etc_prep:
	rm -rf $(INSTALL_ROOT)/etc
	cp -r $(TOPDIR)/filesystem/ap93fus/etc $(INSTALL_ROOT)/

empty_dir_prep:
	if test -d $(INSTALL_ROOT)/mnt; then echo $(INSTALL_ROOT)/mnt exists; else mkdir $(INSTALL_ROOT)/mnt; fi;
	if test -d $(INSTALL_ROOT)/proc; then echo $(INSTALL_ROOT)/proc exists; else mkdir $(INSTALL_ROOT)/proc; fi;
	if test -d $(INSTALL_ROOT)/root; then echo $(INSTALL_ROOT)/root exists; else mkdir $(INSTALL_ROOT)/root; fi;
	if test -d $(INSTALL_ROOT)/tmp; then echo $(INSTALL_ROOT)/mnt exists; else mkdir $(INSTALL_ROOT)/tmp; fi;
	if test -d $(INSTALL_ROOT)/var; then echo $(INSTALL_ROOT)/var exists; else mkdir -p $(INSTALL_ROOT)/var/run; fi;
# added by tf, /sys for mounting sysfs
	if test -d $(INSTALL_ROOT)/sys; then echo $(INSTALL_ROOT)/sys exists; else mkdir $(INSTALL_ROOT)/sys; fi;
	if test -d $(INSTALL_ROOT)/sbin; then echo $(INSTALL_ROOT)/sbin exists; else mkdir $(INSTALL_ROOT)/sbin; fi;
	
CLEANDIR=ap143 pb92 apps kernel_modules 

#build kernel
kernel_clean: 
	cd $(KERNELPATH) &&  $(MAKEARCH) mrproper 

kernel_build:
	@echo " Making Kernel Build Version $(EXTRAVERSION)" at `date`
	cd $(KERNELPATH) && $(MAKEARCH) $(KERNELTARGET)
	cd $(KERNELPATH) && $(MAKEARCH) modules
	cd $(KERNELPATH)/arch/$(KERNELARCH)/boot && \
	cp $(KERNELTARGET) $(TFTPPATH)/$(KTFILE) && \
	cp $(KERNELTARGET) $(IMAGEPATH)/$(KTFILE) && \
	rm -rf $(KERNELTARGET).lzma && \
	$(TOPDIR)/util/lzma/bin/lzma --best --keep $(KERNELTARGET)


#build bootloader
uboot_clean:
	echo Making uboot clean
	cd $(UBOOTDIR) && $(MAKEARCH) distclean


#if you have uboot code,then you will execute uboot_build_copy
#this rule will compile your uboot code and copy uboot to your image direcory
uboot:
	@echo Making uboot at `date`
	cd $(UBOOTDIR) && $(MAKEARCH) mrproper
	cd $(UBOOTDIR) && $(MAKEARCH) $(BOARD_TYPE)_config
	@echo ========= build dir: $(TOPDIR)/build ============
	cd $(UBOOTDIR) && $(MAKEARCH) all BUILD_DIR=$(TOPDIR)/build
	@echo End Making uboot at `date`

####################################### wireless ##########################################

ifeq ($(BUILD_WPA2),y)  ####################################
# apps/wpa2 provides a second generation (as compared with apps/wpa)
#       of authentication (including WPS) programs:
#       hostapd, wpa_supplicant, etc.
# 	It installs via $(INSTALL_ROOT).
# 	It depends only on header files from the driver, and linux driver
# 	(madwifi) header files specified by $(MADWIFIPATH)

wpa2: wpa2_clean #rootfs_prep
	@echo Making wpa2 at `date`
	cd ../$(BOARD_TYPE)/apps/wpa2 && $(MAKE) 	\
		CC=$(TOOLPREFIX)gcc AR=$(TOOLPREFIX)ar LD=$(TOOLPREFIX)ld
	cp $(TOPDIR)/$(BOARD_TYPE)/apps/wpa2/hostapd/hostapd $(WIRELESSPACKAGEPATH)
	cp $(TOPDIR)/$(BOARD_TYPE)/apps/wpa2/common/libwpa_common.so $(WIRELESSPACKAGEPATH)
	cp $(TOPDIR)/$(BOARD_TYPE)/apps/wpa2/common/libwpa_ctrl.so $(WIRELESSPACKAGEPATH)
	cp $(TOPDIR)/$(BOARD_TYPE)/apps/wpa2/wpa_supplicant/wpa_supplicant $(WIRELESSPACKAGEPATH)
	@echo End Making wpa2 at `date`

wpa2_clean:
	cd ../$(BOARD_TYPE)/apps/wpa2 && $(MAKE) clean
clean: wpa2_clean

else  ####(BUILD_WPA2),y####
endif ####(BUILD_WPA2),y####

export WIRELESSPACKAGEPATH=$(IMAGEPATH)/wireless/

wireless_prep:
	test -d $(WIRELESSPACKAGEPATH) || mkdir -p $(WIRELESSPACKAGEPATH)
	
wireless:wireless_prep madwifi_build wpa2

wireless_clean:madwifi_clean wpa2_clean

wireless_tools:
	@echo making wireless_tools 
	if test -n "$(WIRELESSTOOLS)"; then \
	cd $(TOPDIR)/$(BOARD_TYPE)/apps/$(WIRELESSTOOLS) &&  make clean && make; \
	mkdir -p $(INSTALL_ROOT)/sbin; \
	cp -f $(WIRELESSTOOLSLIB) $(WIRELESSPACKAGEPATH); \
	cp -f iwpriv $(WIRELESSPACKAGEPATH); \
	cp -f iwconfig $(WIRELESSPACKAGEPATH); \
	cp -f iwlist $(WIRELESSPACKAGEPATH); \
	for file in $(WIRELESSTOOLNAMES) ; do \
		if [ -f $$file ] ; then \
			cp -f $$file $(INSTALL_ROOT)/sbin/.; fi; done; \
	fi

wireless_tools_clean:
	if test -n "$(WIRELESSTOOLS)"; then \
	cd $(TOPDIR)/$(BOARD_TYPE)/apps/$(WIRELESSTOOLS) &&  make clean; \
	fi	
	
madwifi_build: wireless_tools

madwifi_clean: wireless_tools_clean 

#######################################end wireless ##########################################
	

#all other kernel modules
kernel_modules: kernel_modules_prep $(BOARD_KERNEL_MODULES) 

kernel_modules_prep:
	test -d $(KERNEL_MODULE_PATH) || mkdir -p $(KERNEL_MODULE_PATH)

kernel_modules_install:
	mkdir -p $(INSTALL_ROOT_FOR_BOARD)/lib/modules/$(KERNELVER)/kernel/
	cp -Rf $(KERNEL_MODULE_PATH)/*.ko $(INSTALL_ROOT_FOR_BOARD)/lib/modules/$(KERNELVER)/kernel/
	
netfilter:
	if [ "$(KERNELVER)" = "2.6.15" ]; then \
	$(MAKEARCH) -C $(KERNELPATH) SUBDIRS=net/ipv4/netfilter modules ; \
	else \
	$(MAKEARCH) -C $(KERNELPATH) SUBDIRS=net/netfilter modules ; \
	$(MAKEARCH) -C $(KERNELPATH) SUBDIRS=net/ipv4/netfilter modules ; \
	fi;
	-cp $(KERNELPATH)/net/ipv4/netfilter/*.ko $(KERNEL_MODULE_PATH)
	-cp $(KERNELPATH)/net/netfilter/*.ko $(KERNEL_MODULE_PATH)	

netsched:
	$(MAKEARCH) -C $(KERNELPATH) SUBDIRS=net/sched modules	
	cp $(KERNELPATH)/net/sched/*.ko $(KERNEL_MODULE_PATH)

ts_kmp:
	$(MAKEARCH) -C $(KERNELPATH) SUBDIRS=lib modules	
	cp $(KERNELPATH)/lib/ts_kmp.ko $(KERNEL_MODULE_PATH)
	
pppol2tp:	
	if [ "$(KERNELVER)" = "2.6.15" ]; then \
	$(MAKEARCH) -C $(KERNELPATH) SUBDIRS=$(TOPDIR)/kernel_modules/pppol2tp-kmod-0.17 modules && \
	cp $(TOPDIR)/kernel_modules/pppol2tp-kmod-0.17/*.ko $(KERNEL_MODULE_PATH) ; \
	else \
	$(MAKEARCH) -C $(KERNELPATH) SUBDIRS=drivers/net modules && \
	cp $(KERNELPATH)/drivers/net/pppol2tp.ko $(KERNEL_MODULE_PATH) ; \
	fi;

pptp_module:
	 $(MAKEARCH) -C $(KERNELPATH) SUBDIRS=$(TOPDIR)/kernel_modules/pptp_driver-0.8.3 modules
	cp $(TOPDIR)/kernel_modules/pptp_driver-0.8.3/*.ko $(KERNEL_MODULE_PATH)

#
# applications
#
apps_build: $(BOARD_APPLICATIONS)

busybox_clean:
	@echo Cleaning busybox
	cd ../apps/$(BUSYBOX) && make clean;

busybox: # easy build for busybox, no clean
	cd $(TOPDIR)/apps/$(BUSYBOX); \
	$(MAKE) PREFIX=$(INSTALL_ROOT) uninstall; \
	find . -name "*.o" | xargs rm -rf; \
	$(MAKE); \
	$(MAKE) PREFIX=$(INSTALL_ROOT) install
	
pppoe:
	cd $(TOPDIR)/apps/ppp-2.4.3/pppd && make clean && make CC=$(TOOLPREFIX)gcc LD=$(TOOLPREFIX)ld AR=$(TOOLPREFIX)ar && \
	cp pppd $(INSTALL_ROOT)/usr/sbin

pptp:
	cd $(TOPDIR)/apps/pptp-1.7.1 && make clean && make CC=$(TOOLPREFIX)gcc LD=$(TOOLPREFIX)ld AR=$(TOOLPREFIX)ar && \
	cp pptp $(INSTALL_ROOT)/usr/sbin

l2tp:
	cd $(TOPDIR)/apps/xl2tpd-1.1.12 && make clean &&  make CC=$(TOOLPREFIX)gcc LD=$(TOOLPREFIX)ld AR=$(TOOLPREFIX)ar && \
	cp xl2tpd $(INSTALL_ROOT)/usr/sbin

bpa:
	cd $(TOPDIR)/apps/bpalogin-2.0.2 && make clean &&  make CC=$(TOOLPREFIX)gcc LD=$(TOOLPREFIX)ld AR=$(TOOLPREFIX)ar && \
	cp bpalogin $(INSTALL_ROOT)/usr/sbin
	
iptables:
	cd $(TOPDIR)/apps/iptables-1.4.5 && \
	if test -e Makefile ; then make distclean ; fi && \
	./configure --prefix=$(INSTALL_ROOT) \
	--build=i386 --host=mips-linux  \
	--with-ksource=$(KERNELPATH) \
	--disable-devel --disable-libipq \
	--disable-ipv6 && \
	$(MAKE) CC=$(TOOLPREFIX)gcc LD=$(TOOLPREFIX)ld AR=$(TOOLPREFIX)ar RANLIB=$(TOOLPREFIX)ranlib && \
	$(MAKE) CC=$(TOOLPREFIX)gcc LD=$(TOOLPREFIX)ld AR=$(TOOLPREFIX)ar RANLIB=$(TOOLPREFIX)ranlib install && \
	./rm_ext.sh
	rm -rf $(INSTALL_ROOT)/lib/libexec
	-mv $(INSTALL_ROOT)/libexec $(INSTALL_ROOT)/lib
	rm -rf $(INSTALL_ROOT)/share
	rm -rf $(INSTALL_ROOT)/man
	
tc_build:
	cd $(TOPDIR)/apps/iproute2-$(KERNELVER) && make CC=$(TOOLPREFIX)gcc && \
	cp $(TOPDIR)/apps/iproute2-$(KERNELVER)/tc/tc $(INSTALL_ROOT)/sbin
	
lltd:
	@echo ********** making WinRally ************
	cd $(TOPDIR)/apps/lltd/wr841n-linux && \
	make clean && $(MAKE) CC=$(TOOLPREFIX)gcc LD=$(TOOLPREFIX)gcc && \
	cp lld2d $(INSTALL_ROOT)/usr/bin/

arp:
	cd $(TOPDIR)/apps/arp && \
	$(TOOLPREFIX)gcc -o arp arp.c && cp arp $(INSTALL_ROOT)/usr/

linux_build_all: uboot kernel_build kernel_modules wireless apps_build 
