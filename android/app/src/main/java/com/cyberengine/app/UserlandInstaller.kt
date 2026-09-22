// Cyber Engine :: userland installer
// BusyBox ships as libbusybox.so inside jniLibs/arm64-v8a/ so Android extracts
// it to /data/app/<package>/lib/arm64-v8a/libbusybox.so at install time. That
// directory has the apk_data_file SELinux label and IS EXECUTABLE on stock
// Android (unlike ctx.filesDir which is app_data_file + W^X-denied since API 29).
//
// The PREFIX tree (bin/sbin/usr/home/tmp/etc/var) still lives in ctx.filesDir/linux
// — used for HOME/TMPDIR and as a place for applet symlinks pointing to the
// executable libbusybox.so.
//
// HONESTY (spec §0): this code installs the userland but does NOT claim the
// shell works. Capability stays PARTIAL until a real PTY session runs
// `echo hello` on a real device.
package com.cyberengine.app

import android.content.Context
import java.io.File

class UserlandInstaller(private val ctx: Context) {

    /** Directory Android extracted our libbusybox.so into (executable). */
    val nativeLibDir: File by lazy {
        File(ctx.applicationInfo.nativeLibraryDir ?: "")
    }

    /** Real executable busybox path (inside the apk_data_file labeled dir). */
    val busyboxPath: File by lazy { File(nativeLibDir, "libbusybox.so") }

    /** PREFIX tree at ctx.filesDir/linux (writable, not exec — for HOME/TMPDIR/etc). */
    val prefixDir: File by lazy { File(ctx.filesDir, "linux") }

    /** True if the executable is in place; does NOT mean the shell works. */
    fun ensureInstalled(): Boolean {
        // Build PREFIX tree per spec.
        val dirs = listOf("bin", "sbin", "usr/bin", "usr/sbin", "home", "tmp", "etc", "var", "var/log")
        for (d in dirs) File(prefixDir, d).mkdirs()
        File(prefixDir, "home").mkdirs()
        File(prefixDir, "tmp").mkdirs()

        // Always refresh profile.sh from assets.
        try {
            ctx.assets.open("linux/profile.sh").use { input ->
                File(prefixDir, "etc/profile.sh").also { it.parentFile?.mkdirs() }
                    .outputStream().use { out -> input.copyTo(out) }
            }
        } catch (_: Exception) {}

        // Install applet symlinks pointing to the executable.
        installApplets()

        return busyboxPath.exists() && busyboxPath.canExecute()
    }

    /**
     * Returns true if `libbusybox.so echo hello` produces "hello" on stdout.
     * Runs a real exec — proves the binary is executable in this Android env.
     */
    fun selfTest(): Boolean {
        if (!busyboxPath.exists()) return false
        return try {
            val proc = ProcessBuilder(
                listOf(busyboxPath.absolutePath, "echo", "hello"))
                .redirectErrorStream(true)
                .directory(prefixDir)
                .start()
            val out = proc.inputStream.bufferedReader().readText().trim()
            proc.waitFor()
            out == "hello"
        } catch (e: Exception) {
            android.util.Log.w("UserlandInstaller", "selfTest failed", e)
            false
        }
    }

    /** Returns a multi-line diagnostics string about the executable. */
    fun describeExecutable(): String {
        val p = busyboxPath.absolutePath
        val exists = busyboxPath.exists()
        val size = if (exists) busyboxPath.length() else -1
        val canRead = if (exists) busyboxPath.canRead() else false
        val canExec = if (exists) busyboxPath.canExecute() else false
        val nativeDir = nativeLibDir.absolutePath
        return buildString {
            appendLine("[USERLAND] busybox_path=$p")
            appendLine("[USERLAND] native_lib_dir=$nativeDir")
            appendLine("[USERLAND] exists=$exists size=$size can_read=$canRead can_exec=$canExec")
            appendLine("[USERLAND] prefix_dir=${prefixDir.absolutePath}")
            appendLine("[USERLAND] prefix_writable=${prefixDir.canWrite()}")
            appendLine("[USERLAND] abis=${android.os.Build.SUPPORTED_ABIS.joinToString(",")}")
            appendLine("[USERLAND] sdk=${android.os.Build.VERSION.SDK_INT}")
        }
    }

    private fun installApplets() {
        val binDir = File(prefixDir, "bin")
        val target = busyboxPath.absolutePath  // absolute path to the executable
        // Get applet list from `busybox --list`.
        val applets: List<String> = try {
            val proc = ProcessBuilder(busyboxPath.absolutePath, "--list")
                .redirectErrorStream(true)
                .directory(prefixDir)
                .start()
            val out = proc.inputStream.bufferedReader().readText()
            proc.waitFor()
            out.split("\n").map { it.trim() }.filter { it.isNotEmpty() }
        } catch (e: Exception) {
            android.util.Log.w("UserlandInstaller", "--list failed", e)
            return
        }
        for (applet in applets) {
            val link = File(binDir, applet)
            if (link.exists()) continue
            // Try symlink first (target is absolute path to libbusybox.so).
            try {
                val p = ProcessBuilder("/system/bin/ln", "-s", target, link.absolutePath)
                    .redirectErrorStream(true).start()
                p.waitFor()
                if (link.exists()) continue
            } catch (_: Exception) {}
            // Try hard link.
            try {
                val p = ProcessBuilder("/system/bin/ln", target, link.absolutePath)
                    .redirectErrorStream(true).start()
                p.waitFor()
                if (link.exists()) continue
            } catch (_: Exception) {}
            // Fall back to copy.
            try {
                busyboxPath.copyTo(link, overwrite = true)
                link.setExecutable(true, false)
                link.setReadable(true, false)
            } catch (_: Exception) {}
        }
    }

    fun prefix(): String = prefixDir.absolutePath
}
