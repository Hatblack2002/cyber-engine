// Cyber Engine :: Kotlin JNI surface
package com.cyberengine.app

import android.view.Surface

object CyberBridge {
    init {
        System.loadLibrary("cyberengine_jni")
    }

    external fun nativeEngineCreate(): Long
    external fun nativeEngineDestroy(handle: Long)
    external fun nativeOnPause(handle: Long)
    external fun nativeOnResume(handle: Long)
    external fun nativeOnSurfaceCreated(handle: Long, surface: Surface)
    external fun nativeOnSurfaceDestroyed(handle: Long)
    external fun nativeOnSurfaceResized(handle: Long, w: Int, h: Int)
    external fun nativeRender(handle: Long)
    external fun nativeOrbitCamera(handle: Long, dYaw: Float, dPitch: Float)
    external fun nativeZoomCamera(handle: Long, factor: Float)
    external fun nativePanCamera(handle: Long, dx: Float, dy: Float)
    external fun nativeShellExecute(handle: Long, cmd: String): String
    external fun nativeCapabilities(handle: Long): String
    external fun nativeDiagnostics(handle: Long): String
    external fun nativeRendererFrameCount(handle: Long): Long
    external fun nativeRendererBackend(handle: Long): String

    // Boot diagnostics
    external fun nativeBootLog(handle: Long): String
    external fun nativeBootMark(handle: Long, stage: Int, ok: Boolean, reason: String)

    // Terminal (PTY-backed bundled busybox)
    external fun nativeTerminalStart(busyboxPath: String, prefix: String): Int
    external fun nativeTerminalRead(sid: Int, buf: ByteArray): Int
    external fun nativeTerminalWrite(sid: Int, buf: ByteArray, len: Int): Int
    external fun nativeTerminalStop(sid: Int)
    external fun nativeTerminalResize(sid: Int, rows: Int, cols: Int)
    external fun nativeTerminalSignal(sid: Int, signal: Int)
    external fun nativeTerminalExitStatus(sid: Int): Int
}
