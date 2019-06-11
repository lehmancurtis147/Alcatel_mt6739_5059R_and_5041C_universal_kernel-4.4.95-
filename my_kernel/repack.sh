rm -r -f modules
mkdir -p modules
abootimg -x boot.img bootimg.cfg zImage-dtb
cp ../kernel-4.4.95/OUT/arch/arm/boot/zImage-dtb zImage-dtb
abootimg --create boot_new.img -f bootimg.cfg -k zImage-dtb -r initrd.img
cd ..
cd kernel-4.4.95
find -type f -name *.ko -exec cp '{}' ../my_kernel/modules \;
