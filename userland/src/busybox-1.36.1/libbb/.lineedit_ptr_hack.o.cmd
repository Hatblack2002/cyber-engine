cmd_libbb/lineedit_ptr_hack.o := /home/z/android-sdk/ndk/27.2.12479018/toolchains/llvm/prebuilt/linux-x86_64/bin/aarch64-linux-android34-clang -Wp,-MD,libbb/.lineedit_ptr_hack.o.d  -std=gnu99 -Iinclude -Ilibbb  -include include/autoconf.h -D_GNU_SOURCE -DNDEBUG  -DBB_VER='"1.36.1"' -Wall -Wshadow -Wwrite-strings -Wundef -Wstrict-prototypes -Wunused -Wunused-parameter -Wunused-function -Wunused-value -Wmissing-prototypes -Wmissing-declarations -Wno-format-security -Wdeclaration-after-statement -Wold-style-definition -finline-limit=0 -fno-builtin-strlen -fomit-frame-pointer -ffunction-sections -fdata-sections -funsigned-char -falign-functions=1 -falign-jumps=1 -falign-labels=1 -falign-loops=1 -fno-unwind-tables -fno-asynchronous-unwind-tables -fno-builtin-printf -Oz -static -fPIE -O2   -DKBUILD_BASENAME='"lineedit_ptr_hack"'  -DKBUILD_MODNAME='"lineedit_ptr_hack"' -c -o libbb/lineedit_ptr_hack.o libbb/lineedit_ptr_hack.c

deps_libbb/lineedit_ptr_hack.o := \
  libbb/lineedit_ptr_hack.c \

libbb/lineedit_ptr_hack.o: $(deps_libbb/lineedit_ptr_hack.o)

$(deps_libbb/lineedit_ptr_hack.o):
