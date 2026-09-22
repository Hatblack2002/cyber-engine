cmd_coreutils/tee.o := /home/z/android-sdk/ndk/27.2.12479018/toolchains/llvm/prebuilt/linux-x86_64/bin/aarch64-linux-android34-clang -Wp,-MD,coreutils/.tee.o.d  -std=gnu99 -Iinclude -Ilibbb  -include include/autoconf.h -D_GNU_SOURCE -DNDEBUG  -DBB_VER='"1.36.1"' -Wall -Wshadow -Wwrite-strings -Wundef -Wstrict-prototypes -Wunused -Wunused-parameter -Wunused-function -Wunused-value -Wmissing-prototypes -Wmissing-declarations -Wno-format-security -Wdeclaration-after-statement -Wold-style-definition -finline-limit=0 -fno-builtin-strlen -fomit-frame-pointer -ffunction-sections -fdata-sections -funsigned-char -falign-functions=1 -falign-jumps=1 -falign-labels=1 -falign-loops=1 -fno-unwind-tables -fno-asynchronous-unwind-tables -fno-builtin-printf -Oz -static -fPIE -O2   -DKBUILD_BASENAME='"tee"'  -DKBUILD_MODNAME='"tee"' -c -o coreutils/tee.o coreutils/tee.c

deps_coreutils/tee.o := \
  coreutils/tee.c \
    $(wildcard include/config/tee.h) \
    $(wildcard include/config/feature/tee/use/block/io.h) \
  include/libbb.h \
    $(wildcard include/config/feature/shadowpasswds.h) \
    $(wildcard include/config/use/bb/shadow.h) \
    $(wildcard include/config/selinux.h) \
    $(wildcard include/config/feature/utmp.h) \
    $(wildcard include/config/locale/support.h) \
    $(wildcard include/config/use/bb/pwd/grp.h) \
    $(wildcard include/config/lfs.h) \
    $(wildcard include/config/feature/buffers/go/on/stack.h) \
    $(wildcard include/config/feature/buffers/go/in/bss.h) \
    $(wildcard include/config/extra/cflags.h) \
    $(wildcard include/config/variable/arch/pagesize.h) \
    $(wildcard include/config/feature/verbose.h) \
    $(wildcard include/config/feature/etc/services.h) \
    $(wildcard include/config/feature/ipv6.h) \
    $(wildcard include/config/feature/seamless/xz.h) \
    $(wildcard include/config/feature/seamless/lzma.h) \
    $(wildcard include/config/feature/seamless/bz2.h) \
    $(wildcard include/config/feature/seamless/gz.h) \
    $(wildcard include/config/feature/seamless/z.h) \
    $(wildcard include/config/float/duration.h) \
    $(wildcard include/config/feature/check/names.h) \
    $(wildcard include/config/feature/prefer/applets.h) \
    $(wildcard include/config/long/opts.h) \
    $(wildcard include/config/feature/pidfile.h) \
    $(wildcard include/config/feature/syslog.h) \
    $(wildcard include/config/feature/syslog/info.h) \
    $(wildcard include/config/warn/simple/msg.h) \
    $(wildcard include/config/feature/individual.h) \
    $(wildcard include/config/shell/ash.h) \
    $(wildcard include/config/shell/hush.h) \
    $(wildcard include/config/echo.h) \
    $(wildcard include/config/sleep.h) \
    $(wildcard include/config/printf.h) \
    $(wildcard include/config/test.h) \
    $(wildcard include/config/test1.h) \
    $(wildcard include/config/test2.h) \
    $(wildcard include/config/kill.h) \
    $(wildcard include/config/killall.h) \
    $(wildcard include/config/killall5.h) \
    $(wildcard include/config/chown.h) \
    $(wildcard include/config/ls.h) \
    $(wildcard include/config/xxx.h) \
    $(wildcard include/config/route.h) \
    $(wildcard include/config/feature/hwib.h) \
    $(wildcard include/config/desktop.h) \
    $(wildcard include/config/feature/crond/d.h) \
    $(wildcard include/config/feature/setpriv/capabilities.h) \
    $(wildcard include/config/run/init.h) \
    $(wildcard include/config/feature/securetty.h) \
    $(wildcard include/config/pam.h) \
    $(wildcard include/config/use/bb/crypt.h) \
    $(wildcard include/config/feature/adduser/to/group.h) \
    $(wildcard include/config/feature/del/user/from/group.h) \
    $(wildcard include/config/ioctl/hex2str/error.h) \
    $(wildcard include/config/feature/editing.h) \
    $(wildcard include/config/feature/editing/history.h) \
    $(wildcard include/config/feature/tab/completion.h) \
    $(wildcard include/config/feature/username/completion.h) \
    $(wildcard include/config/feature/editing/fancy/prompt.h) \
    $(wildcard include/config/feature/editing/savehistory.h) \
    $(wildcard include/config/feature/editing/vi.h) \
    $(wildcard include/config/feature/editing/save/on/exit.h) \
    $(wildcard include/config/pmap.h) \
    $(wildcard include/config/feature/show/threads.h) \
    $(wildcard include/config/feature/ps/additional/columns.h) \
    $(wildcard include/config/feature/topmem.h) \
    $(wildcard include/config/feature/top/smp/process.h) \
    $(wildcard include/config/pgrep.h) \
    $(wildcard include/config/pkill.h) \
    $(wildcard include/config/pidof.h) \
    $(wildcard include/config/sestatus.h) \
    $(wildcard include/config/unicode/support.h) \
    $(wildcard include/config/feature/mtab/support.h) \
    $(wildcard include/config/feature/clean/up.h) \
    $(wildcard include/config/feature/devfs.h) \
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
  /home/z/android-sdk/ndk/27.2.12479018/toolchains/llvm/prebuilt/linux-x86_64/bin/../sysroot/usr/include/ctype.h \
  /home/z/android-sdk/ndk/27.2.12479018/toolchains/llvm/prebuilt/linux-x86_64/bin/../sysroot/usr/include/dirent.h \
  /home/z/android-sdk/ndk/27.2.12479018/toolchains/llvm/prebuilt/linux-x86_64/bin/../sysroot/usr/include/errno.h \
  /home/z/android-sdk/ndk/27.2.12479018/toolchains/llvm/prebuilt/linux-x86_64/bin/../sysroot/usr/include/linux/errno.h \
  /home/z/android-sdk/ndk/27.2.12479018/toolchains/llvm/prebuilt/linux-x86_64/bin/../sysroot/usr/include/aarch64-linux-android/asm/errno.h \
  /home/z/android-sdk/ndk/27.2.12479018/toolchains/llvm/prebuilt/linux-x86_64/bin/../sysroot/usr/include/asm-generic/errno.h \
  /home/z/android-sdk/ndk/27.2.12479018/toolchains/llvm/prebuilt/linux-x86_64/bin/../sysroot/usr/include/asm-generic/errno-base.h \
  /home/z/android-sdk/ndk/27.2.12479018/toolchains/llvm/prebuilt/linux-x86_64/bin/../sysroot/usr/include/fcntl.h \
  /home/z/android-sdk/ndk/27.2.12479018/toolchains/llvm/prebuilt/linux-x86_64/bin/../sysroot/usr/include/linux/fadvise.h \
  /home/z/android-sdk/ndk/27.2.12479018/toolchains/llvm/prebuilt/linux-x86_64/bin/../sysroot/usr/include/linux/falloc.h \
  /home/z/android-sdk/ndk/27.2.12479018/toolchains/llvm/prebuilt/linux-x86_64/bin/../sysroot/usr/include/linux/fcntl.h \
  /home/z/android-sdk/ndk/27.2.12479018/toolchains/llvm/prebuilt/linux-x86_64/bin/../sysroot/usr/include/aarch64-linux-android/asm/fcntl.h \
  /home/z/android-sdk/ndk/27.2.12479018/toolchains/llvm/prebuilt/linux-x86_64/bin/../sysroot/usr/include/asm-generic/fcntl.h \
  /home/z/android-sdk/ndk/27.2.12479018/toolchains/llvm/prebuilt/linux-x86_64/bin/../sysroot/usr/include/bits/flock64.h \
  /home/z/android-sdk/ndk/27.2.12479018/toolchains/llvm/prebuilt/linux-x86_64/bin/../sysroot/usr/include/bits/flock.h \
  /home/z/android-sdk/ndk/27.2.12479018/toolchains/llvm/prebuilt/linux-x86_64/bin/../sysroot/usr/include/linux/openat2.h \
  /home/z/android-sdk/ndk/27.2.12479018/toolchains/llvm/prebuilt/linux-x86_64/bin/../sysroot/usr/include/linux/stat.h \
  /home/z/android-sdk/ndk/27.2.12479018/toolchains/llvm/prebuilt/linux-x86_64/bin/../sysroot/usr/include/linux/uio.h \
  /home/z/android-sdk/ndk/27.2.12479018/toolchains/llvm/prebuilt/linux-x86_64/lib/clang/18/include/inttypes.h \
  /home/z/android-sdk/ndk/27.2.12479018/toolchains/llvm/prebuilt/linux-x86_64/bin/../sysroot/usr/include/inttypes.h \
  /home/z/android-sdk/ndk/27.2.12479018/toolchains/llvm/prebuilt/linux-x86_64/bin/../sysroot/usr/include/netdb.h \
  /home/z/android-sdk/ndk/27.2.12479018/toolchains/llvm/prebuilt/linux-x86_64/bin/../sysroot/usr/include/sys/socket.h \
  /home/z/android-sdk/ndk/27.2.12479018/toolchains/llvm/prebuilt/linux-x86_64/bin/../sysroot/usr/include/aarch64-linux-android/asm/socket.h \
  /home/z/android-sdk/ndk/27.2.12479018/toolchains/llvm/prebuilt/linux-x86_64/bin/../sysroot/usr/include/asm-generic/socket.h \
  /home/z/android-sdk/ndk/27.2.12479018/toolchains/llvm/prebuilt/linux-x86_64/bin/../sysroot/usr/include/aarch64-linux-android/asm/sockios.h \
  /home/z/android-sdk/ndk/27.2.12479018/toolchains/llvm/prebuilt/linux-x86_64/bin/../sysroot/usr/include/asm-generic/sockios.h \
  /home/z/android-sdk/ndk/27.2.12479018/toolchains/llvm/prebuilt/linux-x86_64/bin/../sysroot/usr/include/linux/sockios.h \
  /home/z/android-sdk/ndk/27.2.12479018/toolchains/llvm/prebuilt/linux-x86_64/bin/../sysroot/usr/include/bits/sockaddr_storage.h \
  /home/z/android-sdk/ndk/27.2.12479018/toolchains/llvm/prebuilt/linux-x86_64/bin/../sysroot/usr/include/bits/sa_family_t.h \
  /home/z/android-sdk/ndk/27.2.12479018/toolchains/llvm/prebuilt/linux-x86_64/bin/../sysroot/usr/include/setjmp.h \
  /home/z/android-sdk/ndk/27.2.12479018/toolchains/llvm/prebuilt/linux-x86_64/bin/../sysroot/usr/include/paths.h \
  /home/z/android-sdk/ndk/27.2.12479018/toolchains/llvm/prebuilt/linux-x86_64/bin/../sysroot/usr/include/stdio.h \
  /home/z/android-sdk/ndk/27.2.12479018/toolchains/llvm/prebuilt/linux-x86_64/lib/clang/18/include/stdarg.h \
  /home/z/android-sdk/ndk/27.2.12479018/toolchains/llvm/prebuilt/linux-x86_64/lib/clang/18/include/__stdarg___gnuc_va_list.h \
  /home/z/android-sdk/ndk/27.2.12479018/toolchains/llvm/prebuilt/linux-x86_64/lib/clang/18/include/__stdarg_va_list.h \
  /home/z/android-sdk/ndk/27.2.12479018/toolchains/llvm/prebuilt/linux-x86_64/lib/clang/18/include/__stdarg_va_arg.h \
  /home/z/android-sdk/ndk/27.2.12479018/toolchains/llvm/prebuilt/linux-x86_64/lib/clang/18/include/__stdarg___va_copy.h \
  /home/z/android-sdk/ndk/27.2.12479018/toolchains/llvm/prebuilt/linux-x86_64/lib/clang/18/include/__stdarg_va_copy.h \
  /home/z/android-sdk/ndk/27.2.12479018/toolchains/llvm/prebuilt/linux-x86_64/bin/../sysroot/usr/include/stdlib.h \
  /home/z/android-sdk/ndk/27.2.12479018/toolchains/llvm/prebuilt/linux-x86_64/bin/../sysroot/usr/include/alloca.h \
  /home/z/android-sdk/ndk/27.2.12479018/toolchains/llvm/prebuilt/linux-x86_64/bin/../sysroot/usr/include/bits/wait.h \
  /home/z/android-sdk/ndk/27.2.12479018/toolchains/llvm/prebuilt/linux-x86_64/bin/../sysroot/usr/include/linux/wait.h \
  /home/z/android-sdk/ndk/27.2.12479018/toolchains/llvm/prebuilt/linux-x86_64/bin/../sysroot/usr/include/malloc.h \
  /home/z/android-sdk/ndk/27.2.12479018/toolchains/llvm/prebuilt/linux-x86_64/bin/../sysroot/usr/include/android/legacy_stdlib_inlines.h \
  /home/z/android-sdk/ndk/27.2.12479018/toolchains/llvm/prebuilt/linux-x86_64/bin/../sysroot/usr/include/libgen.h \
  /home/z/android-sdk/ndk/27.2.12479018/toolchains/llvm/prebuilt/linux-x86_64/bin/../sysroot/usr/include/poll.h \
  /home/z/android-sdk/ndk/27.2.12479018/toolchains/llvm/prebuilt/linux-x86_64/bin/../sysroot/usr/include/linux/poll.h \
  /home/z/android-sdk/ndk/27.2.12479018/toolchains/llvm/prebuilt/linux-x86_64/bin/../sysroot/usr/include/aarch64-linux-android/asm/poll.h \
  /home/z/android-sdk/ndk/27.2.12479018/toolchains/llvm/prebuilt/linux-x86_64/bin/../sysroot/usr/include/asm-generic/poll.h \
  /home/z/android-sdk/ndk/27.2.12479018/toolchains/llvm/prebuilt/linux-x86_64/bin/../sysroot/usr/include/time.h \
  /home/z/android-sdk/ndk/27.2.12479018/toolchains/llvm/prebuilt/linux-x86_64/bin/../sysroot/usr/include/sys/time.h \
  /home/z/android-sdk/ndk/27.2.12479018/toolchains/llvm/prebuilt/linux-x86_64/bin/../sysroot/usr/include/sys/ioctl.h \
  /home/z/android-sdk/ndk/27.2.12479018/toolchains/llvm/prebuilt/linux-x86_64/bin/../sysroot/usr/include/linux/ioctl.h \
  /home/z/android-sdk/ndk/27.2.12479018/toolchains/llvm/prebuilt/linux-x86_64/bin/../sysroot/usr/include/aarch64-linux-android/asm/ioctl.h \
  /home/z/android-sdk/ndk/27.2.12479018/toolchains/llvm/prebuilt/linux-x86_64/bin/../sysroot/usr/include/asm-generic/ioctl.h \
  /home/z/android-sdk/ndk/27.2.12479018/toolchains/llvm/prebuilt/linux-x86_64/bin/../sysroot/usr/include/linux/termios.h \
  /home/z/android-sdk/ndk/27.2.12479018/toolchains/llvm/prebuilt/linux-x86_64/bin/../sysroot/usr/include/aarch64-linux-android/asm/termios.h \
  /home/z/android-sdk/ndk/27.2.12479018/toolchains/llvm/prebuilt/linux-x86_64/bin/../sysroot/usr/include/asm-generic/termios.h \
  /home/z/android-sdk/ndk/27.2.12479018/toolchains/llvm/prebuilt/linux-x86_64/bin/../sysroot/usr/include/aarch64-linux-android/asm/termbits.h \
  /home/z/android-sdk/ndk/27.2.12479018/toolchains/llvm/prebuilt/linux-x86_64/bin/../sysroot/usr/include/asm-generic/termbits.h \
  /home/z/android-sdk/ndk/27.2.12479018/toolchains/llvm/prebuilt/linux-x86_64/bin/../sysroot/usr/include/asm-generic/termbits-common.h \
  /home/z/android-sdk/ndk/27.2.12479018/toolchains/llvm/prebuilt/linux-x86_64/bin/../sysroot/usr/include/aarch64-linux-android/asm/ioctls.h \
  /home/z/android-sdk/ndk/27.2.12479018/toolchains/llvm/prebuilt/linux-x86_64/bin/../sysroot/usr/include/asm-generic/ioctls.h \
  /home/z/android-sdk/ndk/27.2.12479018/toolchains/llvm/prebuilt/linux-x86_64/bin/../sysroot/usr/include/linux/tty.h \
  /home/z/android-sdk/ndk/27.2.12479018/toolchains/llvm/prebuilt/linux-x86_64/bin/../sysroot/usr/include/sys/mman.h \
  /home/z/android-sdk/ndk/27.2.12479018/toolchains/llvm/prebuilt/linux-x86_64/bin/../sysroot/usr/include/linux/memfd.h \
  /home/z/android-sdk/ndk/27.2.12479018/toolchains/llvm/prebuilt/linux-x86_64/bin/../sysroot/usr/include/asm-generic/hugetlb_encode.h \
  /home/z/android-sdk/ndk/27.2.12479018/toolchains/llvm/prebuilt/linux-x86_64/bin/../sysroot/usr/include/linux/mman.h \
  /home/z/android-sdk/ndk/27.2.12479018/toolchains/llvm/prebuilt/linux-x86_64/bin/../sysroot/usr/include/aarch64-linux-android/asm/mman.h \
  /home/z/android-sdk/ndk/27.2.12479018/toolchains/llvm/prebuilt/linux-x86_64/bin/../sysroot/usr/include/asm-generic/mman.h \
  /home/z/android-sdk/ndk/27.2.12479018/toolchains/llvm/prebuilt/linux-x86_64/bin/../sysroot/usr/include/asm-generic/mman-common.h \
  /home/z/android-sdk/ndk/27.2.12479018/toolchains/llvm/prebuilt/linux-x86_64/bin/../sysroot/usr/include/sys/resource.h \
  /home/z/android-sdk/ndk/27.2.12479018/toolchains/llvm/prebuilt/linux-x86_64/bin/../sysroot/usr/include/linux/resource.h \
  /home/z/android-sdk/ndk/27.2.12479018/toolchains/llvm/prebuilt/linux-x86_64/bin/../sysroot/usr/include/aarch64-linux-android/asm/resource.h \
  /home/z/android-sdk/ndk/27.2.12479018/toolchains/llvm/prebuilt/linux-x86_64/bin/../sysroot/usr/include/asm-generic/resource.h \
  /home/z/android-sdk/ndk/27.2.12479018/toolchains/llvm/prebuilt/linux-x86_64/bin/../sysroot/usr/include/sys/stat.h \
  /home/z/android-sdk/ndk/27.2.12479018/toolchains/llvm/prebuilt/linux-x86_64/bin/../sysroot/usr/include/sys/sysmacros.h \
  /home/z/android-sdk/ndk/27.2.12479018/toolchains/llvm/prebuilt/linux-x86_64/bin/../sysroot/usr/include/sys/wait.h \
  /home/z/android-sdk/ndk/27.2.12479018/toolchains/llvm/prebuilt/linux-x86_64/bin/../sysroot/usr/include/termios.h \
  /home/z/android-sdk/ndk/27.2.12479018/toolchains/llvm/prebuilt/linux-x86_64/bin/../sysroot/usr/include/android/legacy_termios_inlines.h \
  /home/z/android-sdk/ndk/27.2.12479018/toolchains/llvm/prebuilt/linux-x86_64/bin/../sysroot/usr/include/bits/termios_winsize_inlines.h \
  /home/z/android-sdk/ndk/27.2.12479018/toolchains/llvm/prebuilt/linux-x86_64/bin/../sysroot/usr/include/sys/param.h \
  /home/z/android-sdk/ndk/27.2.12479018/toolchains/llvm/prebuilt/linux-x86_64/bin/../sysroot/usr/include/linux/param.h \
  /home/z/android-sdk/ndk/27.2.12479018/toolchains/llvm/prebuilt/linux-x86_64/bin/../sysroot/usr/include/aarch64-linux-android/asm/param.h \
  /home/z/android-sdk/ndk/27.2.12479018/toolchains/llvm/prebuilt/linux-x86_64/bin/../sysroot/usr/include/asm-generic/param.h \
  /home/z/android-sdk/ndk/27.2.12479018/toolchains/llvm/prebuilt/linux-x86_64/bin/../sysroot/usr/include/pwd.h \
  /home/z/android-sdk/ndk/27.2.12479018/toolchains/llvm/prebuilt/linux-x86_64/bin/../sysroot/usr/include/grp.h \
  /home/z/android-sdk/ndk/27.2.12479018/toolchains/llvm/prebuilt/linux-x86_64/bin/../sysroot/usr/include/mntent.h \
  /home/z/android-sdk/ndk/27.2.12479018/toolchains/llvm/prebuilt/linux-x86_64/bin/../sysroot/usr/include/sys/statfs.h \
  /home/z/android-sdk/ndk/27.2.12479018/toolchains/llvm/prebuilt/linux-x86_64/bin/../sysroot/usr/include/sys/vfs.h \
  /home/z/android-sdk/ndk/27.2.12479018/toolchains/llvm/prebuilt/linux-x86_64/bin/../sysroot/usr/include/linux/magic.h \
  /home/z/android-sdk/ndk/27.2.12479018/toolchains/llvm/prebuilt/linux-x86_64/bin/../sysroot/usr/include/arpa/inet.h \
  /home/z/android-sdk/ndk/27.2.12479018/toolchains/llvm/prebuilt/linux-x86_64/bin/../sysroot/usr/include/netinet/in.h \
  /home/z/android-sdk/ndk/27.2.12479018/toolchains/llvm/prebuilt/linux-x86_64/bin/../sysroot/usr/include/netinet/in6.h \
  /home/z/android-sdk/ndk/27.2.12479018/toolchains/llvm/prebuilt/linux-x86_64/bin/../sysroot/usr/include/linux/in6.h \
  /home/z/android-sdk/ndk/27.2.12479018/toolchains/llvm/prebuilt/linux-x86_64/bin/../sysroot/usr/include/linux/libc-compat.h \
  /home/z/android-sdk/ndk/27.2.12479018/toolchains/llvm/prebuilt/linux-x86_64/bin/../sysroot/usr/include/linux/in.h \
  /home/z/android-sdk/ndk/27.2.12479018/toolchains/llvm/prebuilt/linux-x86_64/bin/../sysroot/usr/include/bits/ip_msfilter.h \
  /home/z/android-sdk/ndk/27.2.12479018/toolchains/llvm/prebuilt/linux-x86_64/bin/../sysroot/usr/include/bits/in_addr.h \
  /home/z/android-sdk/ndk/27.2.12479018/toolchains/llvm/prebuilt/linux-x86_64/bin/../sysroot/usr/include/bits/ip_mreq_source.h \
  /home/z/android-sdk/ndk/27.2.12479018/toolchains/llvm/prebuilt/linux-x86_64/bin/../sysroot/usr/include/linux/socket.h \
  /home/z/android-sdk/ndk/27.2.12479018/toolchains/llvm/prebuilt/linux-x86_64/bin/../sysroot/usr/include/aarch64-linux-android/asm/byteorder.h \
  /home/z/android-sdk/ndk/27.2.12479018/toolchains/llvm/prebuilt/linux-x86_64/bin/../sysroot/usr/include/linux/byteorder/little_endian.h \
  /home/z/android-sdk/ndk/27.2.12479018/toolchains/llvm/prebuilt/linux-x86_64/bin/../sysroot/usr/include/linux/swab.h \
  /home/z/android-sdk/ndk/27.2.12479018/toolchains/llvm/prebuilt/linux-x86_64/bin/../sysroot/usr/include/aarch64-linux-android/asm/swab.h \
  /home/z/android-sdk/ndk/27.2.12479018/toolchains/llvm/prebuilt/linux-x86_64/bin/../sysroot/usr/include/asm-generic/swab.h \
  /home/z/android-sdk/ndk/27.2.12479018/toolchains/llvm/prebuilt/linux-x86_64/bin/../sysroot/usr/include/linux/ipv6.h \
  include/xatonum.h \
  include/common_bufsiz.h \

coreutils/tee.o: $(deps_coreutils/tee.o)

$(deps_coreutils/tee.o):
