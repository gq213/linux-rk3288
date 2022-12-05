
1、编译

1)rootfs在flash分区

make rk3288_firefly_defconfig

make zImage -j8

make rk3288-firefly.dtb

2)rootfs在ramdisk里面

make rk3288_firefly_defconfig enable-initrd.config

make zImage -j8

修改dts使用rootfs_in_ramdisk

make rk3288-firefly.dtb


2、使用Micro SD卡启动

格式化Micro SD卡的第2分区

sudo mkfs.vfat /dev/sdb2

拷贝zImage、rk3288-firefly.dtb到该分区


3、使用tftp启动，方便调试

拷贝zImage、rk3288-firefly.dtb到tftp目录

cp arch/arm/boot/zImage ./../

cp arch/arm/boot/dts/rk3288-firefly.dtb ./../
