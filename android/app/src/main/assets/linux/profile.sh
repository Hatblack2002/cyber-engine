// Cyber Engine :: profile.sh — environment for the bundled Linux bridge
// Sourced by ash on startup. Sets PATH and HOME so the userland works like
// Termux (same commands available, no external Termux install required).

export PATH="@PREFIX@/bin:@PREFIX@/sbin:@PREFIX@/usr/bin:/system/bin:/system/xbin"
export HOME="@PREFIX@/home"
export TMPDIR="@PREFIX@/tmp"
export TERM="xterm-256color"
export LANG="C.UTF-8"
export LC_ALL="C.UTF-8"
export CYBER_ENGINE="1"
export CYBER_USERLAND="@PREFIX@"

# Welcome banner
echo "cyber-engine shell (busybox ash) — built-in Linux bridge"
echo "prefix: @PREFIX@"
echo ""

# cd into HOME if it exists
[ -d "$HOME" ] && cd "$HOME" 2>/dev/null || cd "$PREFIX"
