/* This is a generated file, don't edit */

#define NUM_APPLETS 84
#define KNOWN_APPNAME_OFFSETS 4

const uint16_t applet_nameofs[] ALIGN2 = {
109,
223,
353,
};

const char applet_names[] ALIGN1 = ""
"ash" "\0"
"awk" "\0"
"basename" "\0"
"bunzip2" "\0"
"bzcat" "\0"
"cat" "\0"
"clear" "\0"
"comm" "\0"
"cp" "\0"
"cut" "\0"
"date" "\0"
"dd" "\0"
"df" "\0"
"dirname" "\0"
"du" "\0"
"echo" "\0"
"egrep" "\0"
"env" "\0"
"expand" "\0"
"false" "\0"
"fgrep" "\0"
"find" "\0"
"fold" "\0"
"grep" "\0"
"gunzip" "\0"
"gzip" "\0"
"head" "\0"
"hexdump" "\0"
"hostname" "\0"
"id" "\0"
"kill" "\0"
"killall" "\0"
"killall5" "\0"
"ln" "\0"
"ls" "\0"
"md5sum" "\0"
"mkdir" "\0"
"mv" "\0"
"nl" "\0"
"nproc" "\0"
"od" "\0"
"paste" "\0"
"printenv" "\0"
"printf" "\0"
"pwd" "\0"
"readlink" "\0"
"realpath" "\0"
"reset" "\0"
"rm" "\0"
"rmdir" "\0"
"sed" "\0"
"seq" "\0"
"sh" "\0"
"sha1sum" "\0"
"sha256sum" "\0"
"sha512sum" "\0"
"sleep" "\0"
"sort" "\0"
"stat" "\0"
"strings" "\0"
"stty" "\0"
"tac" "\0"
"tail" "\0"
"tar" "\0"
"tee" "\0"
"test" "\0"
"time" "\0"
"touch" "\0"
"tr" "\0"
"true" "\0"
"tty" "\0"
"uname" "\0"
"unexpand" "\0"
"uniq" "\0"
"unxz" "\0"
"usleep" "\0"
"watch" "\0"
"wc" "\0"
"which" "\0"
"whoami" "\0"
"xargs" "\0"
"xzcat" "\0"
"yes" "\0"
"zcat" "\0"
;

#define APPLET_NO_ash 0
#define APPLET_NO_awk 1
#define APPLET_NO_basename 2
#define APPLET_NO_bunzip2 3
#define APPLET_NO_bzcat 4
#define APPLET_NO_cat 5
#define APPLET_NO_clear 6
#define APPLET_NO_comm 7
#define APPLET_NO_cp 8
#define APPLET_NO_cut 9
#define APPLET_NO_date 10
#define APPLET_NO_dd 11
#define APPLET_NO_df 12
#define APPLET_NO_dirname 13
#define APPLET_NO_du 14
#define APPLET_NO_echo 15
#define APPLET_NO_egrep 16
#define APPLET_NO_env 17
#define APPLET_NO_expand 18
#define APPLET_NO_false 19
#define APPLET_NO_fgrep 20
#define APPLET_NO_find 21
#define APPLET_NO_fold 22
#define APPLET_NO_grep 23
#define APPLET_NO_gunzip 24
#define APPLET_NO_gzip 25
#define APPLET_NO_head 26
#define APPLET_NO_hexdump 27
#define APPLET_NO_hostname 28
#define APPLET_NO_id 29
#define APPLET_NO_kill 30
#define APPLET_NO_killall 31
#define APPLET_NO_killall5 32
#define APPLET_NO_ln 33
#define APPLET_NO_ls 34
#define APPLET_NO_md5sum 35
#define APPLET_NO_mkdir 36
#define APPLET_NO_mv 37
#define APPLET_NO_nl 38
#define APPLET_NO_nproc 39
#define APPLET_NO_od 40
#define APPLET_NO_paste 41
#define APPLET_NO_printenv 42
#define APPLET_NO_printf 43
#define APPLET_NO_pwd 44
#define APPLET_NO_readlink 45
#define APPLET_NO_realpath 46
#define APPLET_NO_reset 47
#define APPLET_NO_rm 48
#define APPLET_NO_rmdir 49
#define APPLET_NO_sed 50
#define APPLET_NO_seq 51
#define APPLET_NO_sh 52
#define APPLET_NO_sha1sum 53
#define APPLET_NO_sha256sum 54
#define APPLET_NO_sha512sum 55
#define APPLET_NO_sleep 56
#define APPLET_NO_sort 57
#define APPLET_NO_stat 58
#define APPLET_NO_strings 59
#define APPLET_NO_stty 60
#define APPLET_NO_tac 61
#define APPLET_NO_tail 62
#define APPLET_NO_tar 63
#define APPLET_NO_tee 64
#define APPLET_NO_test 65
#define APPLET_NO_time 66
#define APPLET_NO_touch 67
#define APPLET_NO_tr 68
#define APPLET_NO_true 69
#define APPLET_NO_tty 70
#define APPLET_NO_uname 71
#define APPLET_NO_unexpand 72
#define APPLET_NO_uniq 73
#define APPLET_NO_unxz 74
#define APPLET_NO_usleep 75
#define APPLET_NO_watch 76
#define APPLET_NO_wc 77
#define APPLET_NO_which 78
#define APPLET_NO_whoami 79
#define APPLET_NO_xargs 80
#define APPLET_NO_xzcat 81
#define APPLET_NO_yes 82
#define APPLET_NO_zcat 83

#ifndef SKIP_applet_main
int (*const applet_main[])(int argc, char **argv) = {
ash_main,
awk_main,
basename_main,
bunzip2_main,
bunzip2_main,
cat_main,
clear_main,
comm_main,
cp_main,
cut_main,
date_main,
dd_main,
df_main,
dirname_main,
du_main,
echo_main,
grep_main,
env_main,
expand_main,
false_main,
grep_main,
find_main,
fold_main,
grep_main,
gunzip_main,
gzip_main,
head_main,
hexdump_main,
hostname_main,
id_main,
kill_main,
kill_main,
kill_main,
ln_main,
ls_main,
md5_sha1_sum_main,
mkdir_main,
mv_main,
nl_main,
nproc_main,
od_main,
paste_main,
printenv_main,
printf_main,
pwd_main,
readlink_main,
realpath_main,
reset_main,
rm_main,
rmdir_main,
sed_main,
seq_main,
ash_main,
md5_sha1_sum_main,
md5_sha1_sum_main,
md5_sha1_sum_main,
sleep_main,
sort_main,
stat_main,
strings_main,
stty_main,
tac_main,
tail_main,
tar_main,
tee_main,
test_main,
time_main,
touch_main,
tr_main,
true_main,
tty_main,
uname_main,
expand_main,
uniq_main,
unxz_main,
usleep_main,
watch_main,
wc_main,
which_main,
whoami_main,
xargs_main,
unxz_main,
yes_main,
gunzip_main,
};
#endif

