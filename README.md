
1、编译
1)
默认rootfs在ramdisk里面
make rk3288_firefly_defconfig enable-initrd.config
make zImage -j8
make rk3288-firefly.dtb
2)
rootfs在块设备里面
make rk3288_firefly_defconfig
make zImage -j8
修改dts使用rootfs_in_flash
make rk3288-firefly.dtb

2、编译驱动模块
make modules -j4
make modules_install INSTALL_MOD_PATH=/mnt/disk/nfs/modules

3、使用tftp启动，方便调试
cp arch/arm/boot/zImage ./../tftp
cp arch/arm/boot/dts/rk3288-firefly.dtb ./../tftp
cp drivers/misc/mydrv/mydrv_bt.ko ./../nfs/modules/
cp drivers/net/wireless/broadcom/brcm80211/brcmfmac/brcmfmac.ko ./../nfs/modules/
cp drivers/net/wireless/broadcom/brcm80211/brcmutil/brcmutil.ko ./../nfs/modules/

./../../toolchain/gcc-linaro-6.3.1-2017.05-x86_64_arm-linux-gnueabihf/bin/arm-linux-gnueabihf-strip mydrv_bt.ko --strip-unneeded
./../../toolchain/gcc-linaro-6.3.1-2017.05-x86_64_arm-linux-gnueabihf/bin/arm-linux-gnueabihf-strip brcmfmac.ko --strip-unneeded
./../../toolchain/gcc-linaro-6.3.1-2017.05-x86_64_arm-linux-gnueabihf/bin/arm-linux-gnueabihf-strip brcmutil.ko --strip-unneeded
