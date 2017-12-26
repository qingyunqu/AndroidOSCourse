export LD_LIBRARY_PATH=./android-emulator/linux-x86_64/lib64/qt/lib:$LD_LIBRARY_PATH
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:./android-emulator/linux-x86_64/lib64
./android-emulator/linux-x86_64/emulator64-x86 -kernel ./goldfish/arch/x86_64/boot/bzImage -system ./imgs/system.img -ramdisk ./imgs/ramdisk.img -data ./imgs/userdata.img -sysdir ./imgs/ -show-kernel -verbose -memory 2048 -vendor ./NEXUS6P-vendor.img -gpu off -sdcard ./CFS.img

