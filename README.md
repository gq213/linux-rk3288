
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

2、使用tftp启动，方便调试
cp arch/arm/boot/zImage ./../tftp
cp arch/arm/boot/dts/rk3288-firefly.dtb ./../tftp
