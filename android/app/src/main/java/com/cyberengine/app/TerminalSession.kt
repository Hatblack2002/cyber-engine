// Cyber Engine :: TerminalSession
// Owns the PTY session, output buffer, ANSI parser, and read loop.
// Survives rotation — the View attaches/detaches but the Session persists.
//
// Architecture:
//   TerminalSession (singleton per MainActivity)
//     ├── PTY master fd (via JNI)
//     ├── child PID
//     ├── SpannableStringBuilder (scrollback + ANSI spans)
//     ├── read loop (background thread, sole writer of buffer)
//     └── attachedView (AtomicReference — weak ref to current TerminalView)
//
//   TerminalView (transient — recreated on rotation)
//     └── attach(session) / detach()
//         └── renders session.buffer + forwards input via session.write()
package com.cyberengine.app

import android.graphics.Color
import android.os.Handler
import android.os.Looper
import android.text.SpannableStringBuilder
import android.text.Spanned
import android.text.style.ForegroundColorSpan
import java.util.concurrent.Executors
import java.util.concurrent.atomic.AtomicBoolean
import java.util.concurrent.atomic.AtomicReference

class TerminalSession {

    private val buffer = SpannableStringBuilder()
    private val lock = Any()
    private var sessionId: Int = -1
    private val running = AtomicBoolean(false)
    private val ioExec = Executors.newSingleThreadExecutor()
    private val attachedView = AtomicReference<TerminalView?>(null)
    private val uiHandler = Handler(Looper.getMainLooper())

    private val maxChars = 64 * 1024

    // ANSI parser state — owned by read loop thread only (no sync needed for these)
    private var ansiBuf = StringBuilder()
    private var inEscape = false
    // Pending SGR color to apply to next chars
    private var pendingColor: Int? = null

    fun start(busyboxPath: String, prefix: String): Boolean {
        if (sessionId >= 0) return true
        sessionId = CyberBridge.nativeTerminalStart(busyboxPath, prefix)
        if (sessionId < 0) {
            appendOutput("[ERROR] nativeTerminalStart returned -1 — see boot log\n")
            return false
        }
        running.set(true)
        ioExec.execute {
            val buf = ByteArray(8192)
            while (running.get()) {
                val n = CyberBridge.nativeTerminalRead(sessionId, buf)
                if (n < 0) {
                    val ex = CyberBridge.nativeTerminalExitStatus(sessionId)
                    val code = (ex shr 8) and 0xff
                    val sig = ex and 0xff
                    appendOutput("\n[shell exited: status=$ex exit_code=$code signal=$sig]\n")
                    break
                }
                if (n == 0) continue
                val s = String(buf, 0, n, Charsets.UTF_8)
                appendOutputRaw(s)
            }
            running.set(false)
        }
        return true
    }

    fun stop() {
        running.set(false)
        if (sessionId >= 0) {
            CyberBridge.nativeTerminalStop(sessionId)
            sessionId = -1
        }
    }

    fun isRunning(): Boolean = running.get() && sessionId >= 0

    fun write(bytes: ByteArray) {
        if (sessionId < 0) return
        CyberBridge.nativeTerminalWrite(sessionId, bytes, bytes.size)
    }

    fun sendSignal(signal: Int) {
        if (sessionId >= 0) CyberBridge.nativeTerminalSignal(sessionId, signal)
    }

    fun attachView(view: TerminalView) {
        attachedView.set(view)
        // Sync current buffer state to the new view immediately.
        uiHandler.post {
            renderToView()
        }
    }

    fun detachView(view: TerminalView) {
        attachedView.compareAndSet(view, null)
    }

    /**
     * Public append — used by TerminalView/MainActivity for manual echo of
     * the user's input command (since PTY ECHO is disabled in termios).
     * Thread-safe; can be called from UI thread.
     */
    fun appendOutput(s: String) {
        appendOutputRaw(s)
        notifyView()
    }

    private fun appendOutputRaw(s: String) {
        synchronized(lock) {
            for (ch in s) {
                if (inEscape) {
                    ansiBuf.append(ch)
                    if ((ch in 'a'..'z') || (ch in 'A'..'Z') || ch == '@') {
                        inEscape = false
                        val seq = ansiBuf.toString()
                        ansiBuf.setLength(0)
                        if (seq.endsWith("m")) {
                            applySGR(seq)
                        }
                        // Other CSI sequences (cursor positioning, erase line, etc.)
                        // are STRIPPED — we don't implement them. P01 keeps it simple.
                    }
                } else if (ch == 0x1b.toChar()) {
                    inEscape = true
                    ansiBuf.setLength(0)
                } else if (ch == '\r') {
                    // CR — for simplicity, ignore (next char usually \n)
                } else if (ch == '\n') {
                    buffer.append('\n')
                    // Apply pending color to the newline too (so colored output stays colored)
                    pendingColor?.let { color ->
                        buffer.setSpan(ForegroundColorSpan(color),
                            buffer.length - 1, buffer.length, Spanned.SPAN_EXCLUSIVE_EXCLUSIVE)
                    }
                } else if (ch == 0x08.toChar()) {
                    if (buffer.isNotEmpty()) buffer.delete(buffer.length - 1, buffer.length)
                } else if (ch == 0x07.toChar()) {
                    // BEL — ignore
                } else {
                    val start = buffer.length
                    buffer.append(ch)
                    pendingColor?.let { color ->
                        buffer.setSpan(ForegroundColorSpan(color),
                            start, buffer.length, Spanned.SPAN_EXCLUSIVE_EXCLUSIVE)
                    }
                }
            }
            if (buffer.length > maxChars) {
                buffer.delete(0, buffer.length - maxChars)
            }
        }
    }

    private fun applySGR(seq: String) {
        val body = seq.removePrefix("\u001B[").removeSuffix("m")
        val codes = if (body.isEmpty()) listOf(0) else body.split(";").mapNotNull { it.toIntOrNull() }
        for (c in codes) {
            when (c) {
                0 -> pendingColor = null  // reset
                30 -> pendingColor = Color.rgb(0,0,0)
                31 -> pendingColor = Color.rgb(0xff,0,0)
                32 -> pendingColor = Color.rgb(0,0xff,0)
                33 -> pendingColor = Color.rgb(0xff,0xff,0)
                34 -> pendingColor = Color.rgb(0,0,0xff)
                35 -> pendingColor = Color.rgb(0xff,0,0xff)
                36 -> pendingColor = Color.rgb(0,0xff,0xff)
                37 -> pendingColor = Color.rgb(0xff,0xff,0xff)
                90 -> pendingColor = Color.rgb(0x55,0x55,0x55)
                91 -> pendingColor = Color.rgb(0xff,0x55,0x55)
                92 -> pendingColor = Color.rgb(0x55,0xff,0x55)
                93 -> pendingColor = Color.rgb(0xff,0xff,0x55)
                94 -> pendingColor = Color.rgb(0x55,0x55,0xff)
                95 -> pendingColor = Color.rgb(0xff,0x55,0xff)
                96 -> pendingColor = Color.rgb(0x55,0xff,0xff)
                97 -> pendingColor = Color.rgb(0xff,0xff,0xff)
                // bg colors and other attributes: ignored (P01)
            }
        }
    }

    private fun notifyView() {
        uiHandler.post { renderToView() }
    }

    private fun renderToView() {
        val view = attachedView.get() ?: return
        // TextView.setText(Spanned) takes a snapshot internally; we hold the
        // lock briefly to avoid concurrent mutation during the snapshot.
        synchronized(lock) {
            view.renderBuffer(buffer)
        }
        view.requestScrollToBottom()
    }
}
