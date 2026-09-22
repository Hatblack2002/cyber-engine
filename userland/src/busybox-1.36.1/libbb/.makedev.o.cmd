cmd_libbb/makedev.o := /home/z/android-sdk/ndk/27.2.12479018/toolchains/llvm/prebuilt/linux-x86_64/bin/aarch64-linux-android34-clang -Wp,-MD,libbb/.makedev.o.d  -std=gnu99 -Iinclude -Ilibbb  -include include/autoconf.h -D_GNU_SOURCE -DNDEBUG  -DBB_VER='"1.36.1"' -Wall -Wshadow -Wwrite-strings -Wundef -Wstrict-prototypes -Wunused -Wunused-parameter -Wunused-function -Wunused-value -Wmissing-prototypes -Wmissing-declarations -Wno-format-security -Wdeclaration-after-statement -Wold-style-definition -finline-limit=0 -fno-builtin-strlen -fomit-frame-pointer -ffunction-sections -fdata-sections -funsigned-char -falign-functions=1 -falign-jumps=1 -falign-labels=1 -falign-loops=1 -fno-unwind-tables -fno-asynchronous-unwind-tables -fno-builtin-printf -Oz -static -fPIE -O2   -DKBUILD_BASENAME='"makedev"'  -DKBUILD_MODNAME='"makedev"' -c -o libbb/makedev.o libbb/makedev.c

deps_libbb/makedev.o := \
  libbb/makedev.c \
  include/platform.h \
    $(wildcard include/config/werror.h) \
    $(wildcard include/config/big/endian.h) \
    $(wildcard include/config/little/endian.h) \
    $(wildcard include/config/nommu.h) \
  /home/z/android-sdk/ndk/27.2.12479018/toolchains/llvm/prebuilt/linux-x86_64/lib/clang/18/include/limits.h \
  /home/z/android-sdk/ndk/27.2.12479018/toolchains/llvm/prebuilt/linux-x86_64/bin/../sysroot/usr/include/limits.h \
  /home/z/android-sdk/ndk/27.2.12479018/toolchains/llvm/prebuilt/linux-x86_64/bin/../sysroot/usr/include/sys/cdefs.h \
  /home/z/android-sdk/ndk/27.2.12479018/toolchains/llvm/prebuilt/linux-x86_64/bin/../sysroot/usr/include/android/versioning.h \
  /home/z/android-sdk/ndk/27.2.12479018/toolchains/llvm/prebuilt/linux-x86_64/bin/../sysroot/usr/include/android/api-level.h \
  /home/z/android-sdk/ndk/27.2.12479018/toolchains/llvm/prebuilt/linux-x86_64/bin/../sysroot/usr/include/android/ndk-version.h \
  /home/z/android-sdk/ndk/27.2.12479018/toolchains/llvm/prebuilt/linux-x86_64/lib/clang/18/include/float.h \
  /home/z/android-sdk/ndk/27.2.12479018/toolchains/llvm/prebuilt/linux-x86_64/bin/../sysroot/usr/include/linux/limits.h \
  /home/z/android-sdk/ndk/27.2.12479018/toolchains/llvm/prebuilt/linux-x86_64/bin/../sysroot/usr/include/bits/posix_limits.h \
  /home/z/android-sdk/ndk/27.2.12479018/toolchains/llvm/prebuilt/linux-x86_64/bin/../sysroot/usr/include/byteswap.h \
  /home/z/android-sdk/ndk/27.2.12479018/toolchains/llvm/prebuilt/linux-x86_64/bin/../sysroot/usr/include/sys/endian.h \
  /home/z/android-sdk/ndk/27.2.12479018/toolchains/llvm/prebuilt/linux-x86_64/lib/clang/18/include/stdint.h \
  /home/z/android-sdk/ndk/27.2.12479018/toolchains/llvm/prebuilt/linux-x86_64/bin/../sysroot/usr/include/stdint.h \
  /home/z/android-sdk/ndk/27.2.12479018/toolchains/llvm/prebuilt/linux-x86_64/bin/../sysroot/usr/include/bits/wchar_limits.h \
  /home/z/android-sdk/ndk/27.2.12479018/toolchains/llvm/prebuilt/linux-x86_64/lib/clang/18/include/stddef.h \
  /home/z/android-sdk/ndk/27.2.12479018/toolchains/llvm/prebuilt/linux-x86_64/lib/clang/18/include/__stddef_ptrdiff_t.h \
  /home/z/android-sdk/ndk/27.2.12479018/toolchains/llvm/prebuilt/linux-x86_64/lib/clang/18/include/__stddef_size_t.h \
  /home/z/android-sdk/ndk/27.2.12479018/toolchains/llvm/prebuilt/linux-x86_64/lib/clang/18/include/__stddef_wchar_t.h \
  /home/z/android-sdk/ndk/27.2.12479018/toolchains/llvm/prebuilt/linux-x86_64/lib/clang/18/include/__stddef_null.h \
  /home/z/android-sdk/ndk/27.2.12479018/toolchains/llvm/prebuilt/linux-x86_64/lib/clang/18/include/__stddef_offsetof.h \
  /home/z/android-sdk/ndk/27.2.12479018/toolchains/llvm/prebuilt/linux-x86_64/bin/../sysroot/usr/include/endian.h \
  /home/z/android-sdk/ndk/27.2.12479018/toolchains/llvm/prebuilt/linux-x86_64/lib/clang/18/include/stdbool.h \
  /home/z/android-sdk/ndk/27.2.12479018/toolchains/llvm/prebuilt/linux-x86_64/bin/../sysroot/usr/include/features.h \
  /home/z/android-sdk/ndk/27.2.12479018/toolchains/llvm/prebuilt/linux-x86_64/bin/../sysroot/usr/include/unistd.h \
  /home/z/android-sdk/ndk/27.2.12479018/toolchains/llvm/prebuilt/linux-x86_64/bin/../sysroot/usr/include/sys/types.h \
  /home/z/android-sdk/ndk/27.2.12479018/toolchains/llvm/prebuilt/linux-x86_64/bin/../sysroot/usr/include/linux/types.h \
  /home/z/android-sdk/ndk/27.2.12479018/toolchains/llvm/prebuilt/linux-x86_64/bin/../sysroot/usr/include/aarch64-linux-android/asm/types.h \
  /home/z/android-sdk/ndk/27.2.12479018/toolchains/llvm/prebuilt/linux-x86_64/bin/../sysroot/usr/include/asm-generic/types.h \
  /home/z/android-sdk/ndk/27.2.12479018/toolchains/llvm/prebuilt/linux-x86_64/bin/../sysroot/usr/include/asm-generic/int-ll64.h \
  /home/z/android-sdk/ndk/27.2.12479018/toolchains/llvm/prebuilt/linux-x86_64/bin/../sysroot/usr/include/aarch64-linux-android/asm/bitsperlong.h \
  /home/z/android-sdk/ndk/27.2.12479018/toolchains/llvm/prebuilt/linux-x86_64/bin/../sysroot/usr/include/asm-generic/bitsperlong.h \
  /home/z/android-sdk/ndk/27.2.12479018/toolchains/llvm/prebuilt/linux-x86_64/bin/../sysroot/usr/include/linux/posix_types.h \
  /home/z/android-sdk/ndk/27.2.12479018/toolchains/llvm/prebuilt/linux-x86_64/bin/../sysroot/usr/include/linux/stddef.h \
  /home/z/android-sdk/ndk/27.2.12479018/toolchains/llvm/prebuilt/linux-x86_64/bin/../sysroot/usr/include/linux/compiler_types.h \
  /home/z/android-sdk/ndk/27.2.12479018/toolchains/llvm/prebuilt/linux-x86_64/bin/../sysroot/usr/include/linux/compiler.h \
  /home/z/android-sdk/ndk/27.2.12479018/toolchains/llvm/prebuilt/linux-x86_64/bin/../sysroot/usr/include/aarch64-linux-android/asm/posix_types.h \
  /home/z/android-sdk/ndk/27.2.12479018/toolchains/llvm/prebuilt/linux-x86_64/bin/../sysroot/usr/include/asm-generic/posix_types.h \
  /home/z/android-sdk/ndk/27.2.12479018/toolchains/llvm/prebuilt/linux-x86_64/bin/../sysroot/usr/include/bits/pthread_types.h \
  /home/z/android-sdk/ndk/27.2.12479018/toolchains/llvm/prebuilt/linux-x86_64/bin/../sysroot/usr/include/sys/select.h \
  /home/z/android-sdk/ndk/27.2.12479018/toolchains/llvm/prebuilt/linux-x86_64/bin/../sysroot/usr/include/linux/time.h \
  /home/z/android-sdk/ndk/27.2.12479018/toolchains/llvm/prebuilt/linux-x86_64/bin/../sysroot/usr/include/bits/timespec.h \
  /home/z/android-sdk/ndk/27.2.12479018/toolchains/llvm/prebuilt/linux-x86_64/bin/../sysroot/usr/include/linux/time_types.h \
  /home/z/android-sdk/ndk/27.2.12479018/toolchains/llvm/prebuilt/linux-x86_64/bin/../sysroot/usr/include/signal.h \
  /home/z/android-sdk/ndk/27.2.12479018/toolchains/llvm/prebuilt/linux-x86_64/bin/../sysroot/usr/include/aarch64-linux-android/asm/sigcontext.h \
  /home/z/android-sdk/ndk/27.2.12479018/toolchains/llvm/prebuilt/linux-x86_64/bin/../sysroot/usr/include/aarch64-linux-android/asm/sve_context.h \
  /home/z/android-sdk/ndk/27.2.12479018/toolchains/llvm/prebuilt/linux-x86_64/bin/../sysroot/usr/include/bits/signal_types.h \
  /home/z/android-sdk/ndk/27.2.12479018/toolchains/llvm/prebuilt/linux-x86_64/bin/../sysroot/usr/include/linux/signal.h \
  /home/z/android-sdk/ndk/27.2.12479018/toolchains/llvm/prebuilt/linux-x86_64/bin/../sysroot/usr/include/aarch64-linux-android/asm/signal.h \
  /home/z/android-sdk/ndk/27.2.12479018/toolchains/llvm/prebuilt/linux-x86_64/bin/../sysroot/usr/include/asm-generic/signal.h \
  /home/z/android-sdk/ndk/27.2.12479018/toolchains/llvm/prebuilt/linux-x86_64/bin/../sysroot/usr/include/asm-generic/signal-defs.h \
  /home/z/android-sdk/ndk/27.2.12479018/toolchains/llvm/prebuilt/linux-x86_64/bin/../sysroot/usr/include/aarch64-linux-android/asm/siginfo.h \
  /home/z/android-sdk/ndk/27.2.12479018/toolchains/llvm/prebuilt/linux-x86_64/bin/../sysroot/usr/include/asm-generic/siginfo.h \
  /home/z/android-sdk/ndk/27.2.12479018/toolchains/llvm/prebuilt/linux-x86_64/bin/../sysroot/usr/include/sys/ucontext.h \
  /home/z/android-sdk/ndk/27.2.12479018/toolchains/llvm/prebuilt/linux-x86_64/bin/../sysroot/usr/include/sys/user.h \
  /home/z/android-sdk/ndk/27.2.12479018/toolchains/llvm/prebuilt/linux-x86_64/bin/../sysroot/usr/include/bits/page_size.h \
  /home/z/android-sdk/ndk/27.2.12479018/toolchains/llvm/prebuilt/linux-x86_64/bin/../sysroot/usr/include/bits/fcntl.h \
  /home/z/android-sdk/ndk/27.2.12479018/toolchains/llvm/prebuilt/linux-x86_64/bin/../sysroot/usr/include/bits/getentropy.h \
  /home/z/android-sdk/ndk/27.2.12479018/toolchains/llvm/prebuilt/linux-x86_64/bin/../sysroot/usr/include/bits/getopt.h \
  /home/z/android-sdk/ndk/27.2.12479018/toolchains/llvm/prebuilt/linux-x86_64/bin/../sysroot/usr/include/bits/ioctl.h \
  /home/z/android-sdk/ndk/27.2.12479018/toolchains/llvm/prebuilt/linux-x86_64/bin/../sysroot/usr/include/bits/lockf.h \
  /home/z/android-sdk/ndk/27.2.12479018/toolchains/llvm/prebuilt/linux-x86_64/bin/../sysroot/usr/include/bits/seek_constants.h \
  /home/z/android-sdk/ndk/27.2.12479018/toolchains/llvm/prebuilt/linux-x86_64/bin/../sysroot/usr/include/bits/sysconf.h \
  /home/z/android-sdk/ndk/27.2.12479018/toolchains/llvm/prebuilt/linux-x86_64/bin/../sysroot/usr/include/android/legacy_unistd_inlines.h \
  /home/z/android-sdk/ndk/27.2.12479018/toolchains/llvm/prebuilt/linux-x86_64/bin/../sysroot/usr/include/string.h \
  /home/z/android-sdk/ndk/27.2.12479018/toolchains/llvm/prebuilt/linux-x86_64/bin/../sysroot/usr/include/xlocale.h \
  /home/z/android-sdk/ndk/27.2.12479018/toolchains/llvm/prebuilt/linux-x86_64/bin/../sysroot/usr/include/bits/strcasecmp.h \
  /home/z/android-sdk/ndk/27.2.12479018/toolchains/llvm/prebuilt/linux-x86_64/bin/../sysroot/usr/include/strings.h \
  /home/z/android-sdk/ndk/27.2.12479018/toolchains/llvm/prebuilt/linux-x86_64/bin/../sysroot/usr/include/sys/sysmacros.h \

libbb/makedev.o: $(deps_libbb/makedev.o)

$(deps_libbb/makedev.o):
