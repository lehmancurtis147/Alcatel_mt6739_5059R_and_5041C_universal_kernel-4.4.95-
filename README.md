# Alcatel_1x_5059D_kernel-4.4.95
-------------------------------------------------------------------------
Папку modules из архива ложим по пути drivers/misc/mediatek
В мэйкфайле по пути drivers/misc/mediatek/Makefile в самом низу добавляем

obj-$(CONFIG_MODULES) += modules/connectivity/common/

obj-$(CONFIG_MODULES) += modules/connectivity/bt/mt66xx/      не собрался

obj-$(CONFIG_MODULES) += modules/connectivity/fmradio/

obj-$(CONFIG_MODULES) += modules/connectivity/gps/

obj-$(CONFIG_MODULES) += modules/connectivity/wlan/adaptor/   не собрался

obj-$(CONFIG_MODULES) += modules/connectivity/wlan/core/gen2/ не собрался


модули которые дают ошибку при сборке закамментены в мэйкфайле. Надо разбираться.


Перед сборкой делаем экспорт
export TARGET_BUILD_VARIANT=user

Собираем ядро и потом две команды
mkdir -p modules

find -type f -name *.ko -exec cp '{}' modules \;

Модули в папке modules в корне исходников.

bt_drv.ko

fmradio_drv.ko

gps_drv.ko

wlan_drv.ko

wmt_chrdev_wifi.ko

wmt_drv.ko

Спасибо "vgdn1942" за инструкцию
https://4pda.ru/forum/index.php?showtopic=583114&view=findpost&p=86045582
-------------------------------------------

Папка my_kernel для автоматического переноса ядра в boot и его сборки,
а также для переноса модулей в папку modules.
