arch=x86_64
compiler=/home/lyq/course_file/androidcourse_zyw/x86_64-linux-android-4.9/bin/x86_64-linux-android-
cc=$(compiler)gcc -lm
sysroot=/home/lyq/course_file/androidcourse_zyw/imgs/android-ndk-r13b/platforms/android-21/arch-x86_64
default:
	$(cc) --sysroot=$(sysroot) -pie -fPIE sleeper.c -o sleeper
	$(cc) --sysroot=$(sysroot) -pie -fPIE netuser.c -o netuser
	$(cc) --sysroot=$(sysroot) -pie -fPIE netuser1.c -o netuser1
