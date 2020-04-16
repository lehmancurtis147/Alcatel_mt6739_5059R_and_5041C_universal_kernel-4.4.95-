# WIP DO NOT USE YET !!!!!
# Alcatel Tetra Kernel Source
-------

Before assembly, give the following command.

``` bash
export TARGET_BUILD_VARIANT = user
```

Then build the kernel using this command. 

``` bash
build.sh
```

If the assembly was successful, go to the my_kernel folder, and the working boot.img should be in the folder and run: 

``` bash
repack.sh
```

 Which will take the new boot.img in the my_kernel folder and the modules from the modules folder, and combine them into a finished boot.img. 

-bt_drv.ko
-fmradio_drv.ko
-gps_drv.ko
-wlan_drv.ko
-wmt_chrdev_wifi.ko
-wmt_drv.ko

Thanks to __vgdn194__ for the instruction, and LgPWNd from whom I forked the repo that made this possible.

[REFERENCE](https://w3bsit3-dns.com/forum/index.php?showtopic=583114&view=findpost&p=86045582)

