// Cyber Engine :: MainActivity
// Owns the TerminalSession (survives rotation via onConfigurationChanged —
// MainActivity instance is reused because configChanges is set in manifest).
// On rotation, only the View tree is re-inflated; the Session (PTY, buffer,
// read loop) keeps running. The new TerminalView attaches to the SAME session.
package com.cyberengine.app

import android.app.Activity
import android.content.Context
import android.content.res.Configuration
import android.graphics.Color
import android.os.Bundle
import android.os.StatFs
import android.text.SpannableStringBuilder
import android.text.Spanned
import android.text.style.ForegroundColorSpan
import android.view.Choreographer
import android.view.SurfaceHolder
import android.view.View
import android.view.WindowManager
import android.widget.LinearLayout
import android.widget.ScrollView
import android.widget.TextView
import android.widget.Toast
import java.util.concurrent.Executors
import java.util.concurrent.atomic.AtomicBoolean

class MainActivity : Activity() {

    private var engineHandle: Long = 0L
    private val userland = UserlandInstaller(this)
    private val renderLoop = AtomicBoolean(false)
    private val bgExec = Executors.newSingleThreadExecutor()

    // ===== Session survives rotation (MainActivity instance reused) =====
    private var session: TerminalSession? = null

    private lateinit var surfaceView: CyberSurfaceView
    private lateinit var sidebarLayout: LinearLayout
    private lateinit var capabilitiesText: TextView
    private lateinit var systemText: TextView
    private lateinit var networkText: TextView
    private lateinit var automationText: TextView
    private lateinit var bootText: TextView
    private lateinit var bootScroll: ScrollView
    private lateinit var terminalView: TerminalView
    private lateinit var hudText: TextView

    private var lastX = 0f
    private var lastY = 0f
    private var lastPinchDist = 0f
    private var twoFingerPanLastX = 0f
    private var twoFingerPanLastY = 0f

    private val frameCallback = object : Choreographer.FrameCallback {
        override fun doFrame(frameTimeNanos: Long) {
            if (renderLoop.get()) {
                if (engineHandle != 0L) CyberBridge.nativeRender(engineHandle)
                Choreographer.getInstance().postFrameCallback(this)
                hudText.text = getString(R.string.hud_format).format(
                    CyberBridge.nativeRendererBackend(engineHandle),
                    CyberBridge.nativeRendererFrameCount(engineHandle)
                )
            }
        }
    }

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        CyberBridge.nativeBootMark(0L, 0, true, "MainActivity onCreate")
        window.addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON)
        try {
            applyLayoutForOrientation()
            setupSidebar()
            setupSurface()
            bindTerminalView()  // attaches the new view to the existing session (if any)

            // First-time session creation ONLY (not on rotation — onConfigurationChanged
            // reuses this same MainActivity instance).
            if (session == null) {
                val ok = userland.ensureInstalled()
                android.util.Log.i("CyberMain", "userland installed: $ok path=${userland.prefix()}")
                CyberBridge.nativeBootMark(engineHandle, 9, ok, "prefix=${userland.prefix()}")

                engineHandle = CyberBridge.nativeEngineCreate()
                if (engineHandle == 0L) {
                    Toast.makeText(this, "Engine boot failed — see boot log", Toast.LENGTH_LONG).show()
                }

                if (ok && engineHandle != 0L) {
                    val s = TerminalSession()
                    if (s.start(userland.busyboxPath.absolutePath, userland.prefix())) {
                        session = s
                    } else {
                        Toast.makeText(this, "Terminal session start failed — see boot log",
                            Toast.LENGTH_LONG).show()
                    }
                }
            }

            // Attach the existing (or just-created) session to the current view.
            session?.let { sess ->
                terminalView.attach(sess)
                terminalView.setCommandHandler { cmd ->
                    handleCommand(cmd)
                }
                terminalView.requestInputFocus()
            }

            refreshPanels()
        } catch (t: Throwable) {
            android.util.Log.e("CyberMain", "onCreate crash", t)
            CyberBridge.nativeBootMark(engineHandle, 0, false, "${t.javaClass.simpleName}: ${t.message}")
            Toast.makeText(this, "Crash: ${t.javaClass.simpleName}: ${t.message}", Toast.LENGTH_LONG).show()
        }
    }

    override fun onConfigurationChanged(newConfig: Configuration) {
        super.onConfigurationChanged(newConfig)
        // ===== PRESERVE SESSION ACROSS ROTATION =====
        // The PTY, child PID, buffer, and read loop all live in `session` —
        // we DON'T stop or restart it. Only the View tree is re-inflated.
        if (::terminalView.isInitialized) {
            terminalView.detach()  // disconnect old view from session
        }
        applyLayoutForOrientation()
        setupSidebar()
        setupSurface()
        bindTerminalView()  // find new terminalView, attach to SAME session
        refreshPanels()
        // The new view re-syncs the existing scrollback via TerminalSession.attachView().
    }

    private fun applyLayoutForOrientation() {
        setContentView(R.layout.main)
        bindViews()
    }

    private fun bindViews() {
        surfaceView = findViewById(R.id.surfaceView)
        sidebarLayout = findViewById(R.id.sidebar)
        capabilitiesText = findViewById(R.id.capabilitiesText)
        systemText = findViewById(R.id.systemText)
        networkText = findViewById(R.id.networkText)
        automationText = findViewById(R.id.automationText)
        terminalView = findViewById(R.id.terminalView)
        hudText = findViewById(R.id.hud)
        bootText = findViewById(R.id.bootText)
        val bs: View? = findViewById(R.id.bootScroll)
        bootScroll = bs as? ScrollView
            ?: bootText.parent as? ScrollView
            ?: ScrollView(this)
    }

    private fun setupSidebar() {
        sidebarLayout.removeAllViews()
        val items = listOf(
            "inicio" to "INICIO",
            "viewport" to "VIEWPORT",
            "terminal" to "TERMINAL",
            "capabilities" to "CAPS",
            "system" to "SISTEMA",
            "diagnostics" to "DIAG",
            "boot" to "BOOT"
        )
        for ((id, label) in items) {
            val b = TextView(this).apply {
                text = label
                setTextColor(Color.CYAN)
                setPadding(20, 16, 20, 16)
                setOnClickListener {
                    when (id) {
                        "terminal" -> terminalView.requestInputFocus()
                        "boot" -> {
                            bootText.text = CyberBridge.nativeBootLog(engineHandle)
                            if (::bootScroll.isInitialized) {
                                bootScroll.visibility =
                                    if (bootScroll.visibility == View.VISIBLE) View.GONE else View.VISIBLE
                            }
                        }
                        else -> refreshPanels()
                    }
                }
            }
            sidebarLayout.addView(b)
        }
    }

    private fun setupSurface() {
        surfaceView.holder.addCallback(object : SurfaceHolder.Callback {
            override fun surfaceCreated(holder: SurfaceHolder) {
                if (engineHandle != 0L) {
                    CyberBridge.nativeOnSurfaceCreated(engineHandle, holder.surface)
                }
                renderLoop.set(true)
                Choreographer.getInstance().postFrameCallback(frameCallback)
            }
            override fun surfaceChanged(holder: SurfaceHolder, format: Int, w: Int, h: Int) {
                if (engineHandle != 0L) CyberBridge.nativeOnSurfaceResized(engineHandle, w, h)
            }
            override fun surfaceDestroyed(holder: SurfaceHolder) {
                renderLoop.set(false)
                Choreographer.getInstance().removeFrameCallback(frameCallback)
                if (engineHandle != 0L) CyberBridge.nativeOnSurfaceDestroyed(engineHandle)
            }
        })
        surfaceView.setOnTouchListener { _, ev ->
            when (ev.actionMasked) {
                android.view.MotionEvent.ACTION_DOWN -> {
                    lastX = ev.x; lastY = ev.y
                }
                android.view.MotionEvent.ACTION_POINTER_DOWN -> {
                    if (ev.pointerCount == 2) {
                        lastPinchDist = distance(ev)
                        twoFingerPanLastX = (ev.getX(0) + ev.getX(1)) / 2
                        twoFingerPanLastY = (ev.getY(0) + ev.getY(1)) / 2
                    }
                }
                android.view.MotionEvent.ACTION_MOVE -> {
                    if (ev.pointerCount == 1) {
                        val dx = ev.x - lastX
                        val dy = ev.y - lastY
                        if (engineHandle != 0L) {
                            CyberBridge.nativeOrbitCamera(engineHandle, dx * 0.01f, dy * 0.01f)
                        }
                        lastX = ev.x; lastY = ev.y
                    } else if (ev.pointerCount == 2) {
                        val d = distance(ev)
                        if (lastPinchDist > 0 && engineHandle != 0L) {
                            val factor = lastPinchDist / d
                            CyberBridge.nativeZoomCamera(engineHandle, factor.coerceIn(0.5f, 2.0f))
                        }
                        val cx = (ev.getX(0) + ev.getX(1)) / 2
                        val cy = (ev.getY(0) + ev.getY(1)) / 2
                        if (engineHandle != 0L && twoFingerPanLastX != 0f) {
                            CyberBridge.nativePanCamera(engineHandle,
                                (cx - twoFingerPanLastX) * 0.01f,
                                (cy - twoFingerPanLastY) * 0.01f)
                        }
                        twoFingerPanLastX = cx; twoFingerPanLastY = cy
                        lastPinchDist = d
                    }
                }
                android.view.MotionEvent.ACTION_UP, android.view.MotionEvent.ACTION_CANCEL,
                android.view.MotionEvent.ACTION_POINTER_UP -> {
                    if (ev.pointerCount <= 1) {
                        lastPinchDist = 0f
                        twoFingerPanLastX = 0f; twoFingerPanLastY = 0f
                    }
                }
            }
            true
        }
    }

    private fun bindTerminalView() {
        // Find the terminalView from the freshly inflated layout.
        // The session is NOT recreated here — it persists from onCreate.
        // The new view will be attached to the existing session in the caller.
    }

    /**
     * Command handler — decides cyber shell builtins vs PTY (busybox ash).
     * Builtins run in C++ engine via nativeShellExecute (no PTY round-trip).
     * Non-builtins are sent to the PTY as `cmd\r` (the shell processes them).
     * Manual echo of `cmd\n` is already done by TerminalView.dispatchEnter
     * BEFORE this handler runs — so the user sees the command in the buffer
     * exactly once, regardless of which path it takes.
     *
     * NOTE: `echo` is NOT a builtin — busybox echo supports redirection
     * (`echo cyber > test.txt`), the cyber shell echo does not. So `echo`
     * goes to the PTY for full shell semantics.
     */
    private fun handleCommand(cmd: String) {
        val s = session ?: return
        val trimmed = cmd.trim()
        if (trimmed.isEmpty()) {
            // Empty Enter — send just CR so the shell prints a fresh prompt.
            s.write(byteArrayOf(0x0d))
            return
        }
        val builtins = setOf("help", "version", "capabilities", "diagnostics", "clear")
        val first = trimmed.split(" ").firstOrNull() ?: ""
        if (builtins.contains(first)) {
            // Cyber shell builtin — run in engine, append output.
            val out = CyberBridge.nativeShellExecute(engineHandle, cmd)
            s.appendOutput(out)
            // Trigger a fresh prompt from the shell (send empty line).
            s.write(byteArrayOf(0x0d))
        } else {
            // PTY command — send `cmd\r` to busybox ash.
            // ICRNL in termios maps CR → NL so the shell sees `cmd\n`.
            s.write((cmd + "\r").toByteArray(Charsets.UTF_8))
        }
    }

    private fun refreshPanels() {
        bgExec.execute {
            val caps = if (engineHandle != 0L) CyberBridge.nativeCapabilities(engineHandle) else ""
            val boot = if (engineHandle != 0L) CyberBridge.nativeBootLog(engineHandle) else ""
            val sys  = readRealSystemInfo()
            runOnUiThread {
                capabilitiesText.text = formatCaps(caps)
                systemText.text = sys
                bootText.text = boot
                networkText.text = "NETWORK.OBSERVATION  NOT_IMPLEMENTED  (spec §0 forbids faking)"
                automationText.text = "AUTOMATION  NOT_IMPLEMENTED  (Lua sandbox pending — spec §10)"
            }
        }
    }

    private fun formatCaps(raw: String): CharSequence {
        val sb = SpannableStringBuilder()
        sb.append("CAPABILITY          STATE                NOTE\n")
        sb.append("-----------------------------------------\n")
        for (line in raw.split("\n").filter { it.isNotEmpty() }) {
            val parts = line.split("|", limit = 3)
            if (parts.size != 3) continue
            sb.append(parts[0].padEnd(17))
            val state = parts[1]
            val start = sb.length
            sb.append(state.padEnd(21))
            val color = when (state) {
                "AVAILABLE" -> Color.rgb(0x00, 0xff, 0x41)
                "UNAVAILABLE", "NOT_IMPLEMENTED" -> Color.rgb(0xff, 0x00, 0x55)
                "REQUIRES_ROOT", "REQUIRES_PERMISSION", "REQUIRES_HARDWARE" -> Color.rgb(0xff, 0xaa, 0x00)
                "PARTIAL" -> Color.rgb(0xff, 0xaa, 0x00)
                else -> Color.WHITE
            }
            sb.setSpan(ForegroundColorSpan(color), start, sb.length, Spanned.SPAN_EXCLUSIVE_EXCLUSIVE)
            sb.append(parts[2])
            sb.append("\n")
        }
        return sb
    }

    private fun readRealSystemInfo(): String {
        val am = getSystemService(Context.ACTIVITY_SERVICE) as android.app.ActivityManager
        val mi = android.app.ActivityManager.MemoryInfo().also { am.getMemoryInfo(it) }
        val totalRamMb = mi.totalMem / (1024 * 1024)
        val availRamMb = mi.availMem / (1024 * 1024)
        val usedRamMb = totalRamMb - availRamMb
        val bm = getSystemService(Context.BATTERY_SERVICE) as android.os.BatteryManager
        val battPct = bm.getIntProperty(android.os.BatteryManager.BATTERY_PROPERTY_CAPACITY)

        val filesDirAbs = filesDir.absolutePath
        val stat = StatFs(filesDirAbs)
        val totalBytes = stat.totalBytes
        val availBytes = stat.availableBytes
        val usedBytes = totalBytes - availBytes
        val totalMB = totalBytes / (1024 * 1024)
        val availMB = availBytes / (1024 * 1024)
        val usedMB = usedBytes / (1024 * 1024)

        val sb = StringBuilder()
        sb.append("SISTEMA — datos reales (Android APIs)\n")
        sb.append("--------------------------------------\n")
        sb.append("RAM total:     $totalRamMb MB\n")
        sb.append("RAM usada:     $usedRamMb MB\n")
        sb.append("RAM libre:     $availRamMb MB\n")
        sb.append("Batería:       $battPct%\n")
        sb.append("Procesadores:  ${Runtime.getRuntime().availableProcessors()}\n")
        sb.append("Storage total: $totalMB MB (app-private)\n")
        sb.append("Storage usado: $usedMB MB\n")
        sb.append("Storage libre: $availMB MB\n")
        sb.append("ABI:           ${android.os.Build.SUPPORTED_ABIS.joinToString(",")}\n")
        sb.append("SDK nivel:     ${android.os.Build.VERSION.SDK_INT}\n")
        sb.append("Dispositivo:   ${android.os.Build.MANUFACTURER} ${android.os.Build.MODEL}\n")
        sb.append("\n")
        sb.append("(CPU %: NOT_IMPLEMENTED — no API pública confiable)\n")
        sb.append("(Temperatura: NOT_IMPLEMENTED — sin API pública estable)\n")
        sb.append("(Red: NOT_IMPLEMENTED — spec §0 prohíbe fingir)\n")
        sb.append("(Procesos: REQUIRES_ROOT — /proc completo requiere root)\n")
        return sb.toString()
    }

    override fun onPause() {
        super.onPause()
        renderLoop.set(false)
        Choreographer.getInstance().removeFrameCallback(frameCallback)
        if (engineHandle != 0L) CyberBridge.nativeOnPause(engineHandle)
        // Session keeps running — don't stop it. Just detach the view.
        if (::terminalView.isInitialized) terminalView.detach()
    }

    override fun onResume() {
        super.onResume()
        if (engineHandle != 0L) CyberBridge.nativeOnResume(engineHandle)
        // Re-attach the view to the persistent session.
        if (::terminalView.isInitialized && session != null) {
            terminalView.attach(session!!)
            terminalView.setCommandHandler { cmd -> handleCommand(cmd) }
            terminalView.requestInputFocus()
        }
        Choreographer.getInstance().postFrameCallback(frameCallback)
    }

    override fun onDestroy() {
        super.onDestroy()
        renderLoop.set(false)
        // Only stop the session when the Activity is truly destroyed
        // (not on rotation — onConfigurationChanged doesn't call onDestroy).
        session?.stop()
        session = null
        if (engineHandle != 0L) CyberBridge.nativeEngineDestroy(engineHandle)
        bgExec.shutdown()
    }

    private fun distance(ev: android.view.MotionEvent): Float {
        val dx = ev.getX(0) - ev.getX(1)
        val dy = ev.getY(0) - ev.getY(1)
        return kotlin.math.sqrt(dx * dx + dy * dy)
    }
}
