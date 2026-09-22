#!/bin/bash
# Cross-compile busybox static for Android arm64.
# Strategy: allnoconfig (all set to 'no' explicitly, no NEW symbols) -> enable
# what we need via .config edits -> wait 1s -> make (silentoldconfig runs
# but finds no unresolved symbols so no prompts).
set -uo pipefail
source /home/z/.android-env.sh

NDK_TOOLCHAIN=$ANDROID_NDK_HOME/toolchains/llvm/prebuilt/linux-x86_64
NDK_CC=$NDK_TOOLCHAIN/bin/aarch64-linux-android34-clang
NDK_AR=$NDK_TOOLCHAIN/bin/llvm-ar
NDK_RANLIB=$NDK_TOOLCHAIN/bin/llvm-ranlib

SRC=/home/z/my-project/cyber-engine/userland/src/busybox-1.36.1
OUT=/home/z/my-project/cyber-engine/userland/binaries
mkdir -p $OUT

cd $SRC
make clean 2>/dev/null || true

# Step 1: allnoconfig — produces .config with all symbols explicitly set to no.
make ARCH=arm64 CC=$NDK_CC AR=$NDK_AR RANLIB=$NDK_RANLIB allnoconfig > /dev/null 2>&1 || true

# Verify .config has explicit "no" entries (canonical form: "# CONFIG_X is not set")
head -10 .config
echo "total symbols: $(wc -l < .config)"

# Helper: enable a symbol
y() {
    local sym=$1 val=${2:-y}
    sed -i "/^# CONFIG_${sym} is not set\$/d; /^CONFIG_${sym}=/d" .config
    echo "CONFIG_${sym}=${val}" >> .config
}

# Step 2: enable what we need.
y STATIC
y HAVE_DOT_CONFIG
y DESKTOP
y ASH
y ASH_ALIAS
y ASH_BUILTIN_ECHO
y ASH_BUILTIN_PRINTF
y ASH_BUILTIN_TEST
y ASH_MATH_SUPPORT
y ASH_MATH_SUPPORT_64
y ASH_RANDOM_SUPPORT
y SH_IS_ASH
y BASH_IS_ASH
y FEATURE_EDITING
y FEATURE_EDITING_HISTORY 256
y FEATURE_EDITING_MAX_LEN 1024
y FEATURE_TAB_COMPLETION
y FEATURE_USERNAME_COMPLETION
y FEATURE_FANCY_ECHO
y FEATURE_TEST_64
y ECHO
y PRINTF
y TEST
y TRUE
y FALSE
y YES
y CAT
y HEAD
y TAIL
y WC
y GREP
y EGREP
y FGREP
y SED
y AWK
y TR
y CUT
y PASTE
y SORT
y UNIQ
y TEE
y FIND
y XARGS
y LS
y COLOR_LS
y DIRNAME
y BASENAME
y MKDIR
y RMDIR
y MV
y CP
y RM
y LN
y TOUCH
y STAT
y DD
y DF
y DU
y PWD
y ENV
y PRINTENV
y REALPATH
y READLINK
y SLEEP
y DATE
y HOSTNAME
y UNAME
y WHOAMI
y ID
y CLEAR
y RESET
y STTY
y TTY
y WHICH
y WHEREIS
y EXPAND
y UNEXPAND
y SEQ
y TAC
y OD
y HEXDUMP
y MD5SUM
y SHA1SUM
y SHA256SUM
y SHA512SUM
y TAR
y GZIP
y GUNZIP
y BUNZIP2
y BZCAT
y UNXZ
y XZCAT
y ZCAT
y FILE
y STRINGS
y WATCH
y TIME
y KILL
y KILLALL
y KILLALL5
y NPROC
y NL
y COMM
y FOLD
y USLEEP
y FEATURE_LS_COLOR
y FEATURE_LS_TIMESTAMPS
y FEATURE_LS_FILETYPES
y FEATURE_LS_RECURSIVE
y FEATURE_LS_FOLLOWLINKS
y FEATURE_VI
y FEATURE_VI_COLON
y FEATURE_VI_YANKMARK
y FEATURE_VI_SEARCH
y FEATURE_VI_USE_SIGNALS
y FEATURE_VI_DOT_CMD
y FEATURE_VI_READONLY
y FEATURE_VI_SETOPTS
y FEATURE_VI_SET
y FEATURE_VI_WIN_RESIZE
y FEATURE_VI_8BIT
y FEATURE_VI_MAX_LEN 4096
y FEATURE_ALLOW_EXEC
y FEATURE_PRESERVE_HARDLINKS
y FEATURE_VERBOSE
y FEATURE_HUMAN_READABLE
y FEATURE_USE_PRINTINT
y FEATURE_CP_REFLINK_AUTO

# Step 3: regenerate autoconf.h non-interactively via pty trick.
# silentoldconfig detects redirected stdin/stdout and refuses to run, but
# `script -q` gives it a pty; feeding 2000 newlines means "use default"
# for any new/unresolved symbol (all our explicit choices are in .config).
echo "=== Regenerating autoconf.h via pty ==="
yes "" | head -2000 | script -q -c \
    "make ARCH=arm64 CC=$NDK_CC AR=$NDK_AR RANLIB=$NDK_RANLIB silentoldconfig" \
    /dev/null > /tmp/bb_kc.log 2>&1 || true
ls -la include/autoconf.h
# Confirm HOSTID and LOADFONT are off
grep -E "ENABLE_HOSTID|ENABLE_LOADFONT" include/autoconf.h | head -3

# Make autoconf.h newer than .config so the build skips re-running kconfig.
touch include/autoconf.h

# Step 4: build.
echo "=== Building busybox ==="
make ARCH=arm64 \
     CC=$NDK_CC \
     AR=$NDK_AR \
     RANLIB=$NDK_RANLIB \
     CROSS_COMPILE= \
     EXTRA_CFLAGS="-static -fPIE -O2" \
     EXTRA_LDFLAGS="-static -pie -Wl,--allow-multiple-definition" \
     -j2 > /tmp/bb_final.log 2>&1
echo "exit=$?"
tail -5 /tmp/bb_final.log
echo "=== errors? ==="
grep -iE "fatal|error" /tmp/bb_final.log | head -10
echo "=== result ==="
ls -la busybox_unstripped 2>&1
file busybox_unstripped 2>&1
if [ -f busybox_unstripped ]; then
    cp -f busybox_unstripped $OUT/busybox
    $NDK_TOOLCHAIN/bin/llvm-strip $OUT/busybox 2>/dev/null || true
    ls -la $OUT/busybox
    file $OUT/busybox
fi
