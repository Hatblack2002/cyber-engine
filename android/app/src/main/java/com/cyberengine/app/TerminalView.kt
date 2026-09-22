// Cyber Engine :: TerminalView (thin renderer + input handler)
// Lifecycle:
//   - created by layout inflation
//   - attaches to a TerminalSession (which survives rotation)
//   - on detach, just disconnects; session keeps running
//
// Input model:
//   - User types in CyberEditText (live visual feedback)
//   - Keyboard Enter: fires IME_ACTION_SEND (setOnEditorActionListener) OR
//     inserts '\n' which TextWatcher catches → dispatchEnter()
//   - Paste (single-line or multi-line): CyberEditText.onTextContextMenuItem
//     sets isPasteOperation=true → TextWatcher skips dispatch → full content
//     stays in EditText → user presses Enter manually to execute
//   - On Enter with multi-line content: dispatchEnter takes the FIRST line
//     as `cmd`, keeps the rest in EditText for subsequent Enters
//
// DIFFERENTIATION:
//   - Keyboard Enter → dispatches immediately (one cmd per Enter)
//   - Paste → NEVER auto-dispatches (user must press Enter to run)
//
// No live char forwarding to PTY (was the original duplicate bug).
// No second session on rotation (session owned by MainActivity).
package com.cyberengine.app

import android.content.Context
import android.graphics.Color
import android.graphics.Typeface
import android.os.Handler
import android.os.Looper
import android.text.Editable
import android.text.InputType
import android.text.SpannableStringBuilder
import android.text.TextWatcher
import android.util.AttributeSet
import android.view.KeyEvent
import android.view.inputmethod.EditorInfo
import android.widget.LinearLayout
import android.widget.ScrollView
import android.widget.TextView

class TerminalView @JvmOverloads constructor(
    ctx: Context, attrs: AttributeSet? = null
) : LinearLayout(ctx, attrs) {

    private val outputView = TextView(ctx).apply {
        typeface = Typeface.MONOSPACE
        setTextColor(Color.rgb(0x00, 0xff, 0x41))
        textSize = 11f
        setPadding(16, 8, 16, 8)
        setTextIsSelectable(true)
    }
    private val inputView = CyberEditText(ctx).apply {
        typeface = Typeface.MONOSPACE
        hint = "input (Enter to run)"
        setHintTextColor(Color.rgb(0x40, 0x80, 0x80))
        setTextColor(Color.rgb(0x00, 0xff, 0x41))
        setBackgroundColor(Color.argb(40, 0, 0, 0))
        setPadding(16, 8, 16, 8)
        // isSingleLine = false so multi-line PASTE content is preserved
        // (the IME would strip '\n' if isSingleLine=true).
        // Keyboard Enter still fires IME_ACTION_SEND (because imeOptions is set);
        // if the IME inserts '\n' instead, TextWatcher catches it.
        inputType = InputType.TYPE_CLASS_TEXT
        imeOptions = EditorInfo.IME_ACTION_SEND or EditorInfo.IME_FLAG_NO_EXTRACT_UI
        isSingleLine = false
        isFocusable = true
        isFocusableInTouchMode = true
        setOnClickListener { requestFocus() }
    }
    private val scrollView = ScrollView(ctx).apply { addView(outputView) }

    private var session: TerminalSession? = null
    private var commandHandler: ((String) -> Unit)? = null

    @Volatile private var isDispatching: Boolean = false
    private val uiHandler = Handler(Looper.getMainLooper())

    init {
        orientation = VERTICAL
        addView(scrollView, LayoutParams(LayoutParams.MATCH_PARENT, 0).apply { weight = 1f })
        addView(inputView, LayoutParams(LayoutParams.MATCH_PARENT, LayoutParams.WRAP_CONTENT))

        // ===== PRIMARY Enter handler: IME action (soft keyboard "Send") =====
        inputView.setOnEditorActionListener { _, actionId, _ ->
            if (actionId == EditorInfo.IME_ACTION_SEND
                || actionId == EditorInfo.IME_ACTION_DONE
                || actionId == EditorInfo.IME_ACTION_GO
                || actionId == EditorInfo.IME_ACTION_NEXT) {
                dispatchEnter(); true
            } else false
        }

        // ===== HARDWARE key handler (physical keyboards) =====
        inputView.setOnKeyListener { _, keyCode, ev ->
            if (ev.action != KeyEvent.ACTION_DOWN) return@setOnKeyListener false
            when (keyCode) {
                KeyEvent.KEYCODE_ENTER, KeyEvent.KEYCODE_NUMPAD_ENTER -> {
                    dispatchEnter(); true
                }
                KeyEvent.KEYCODE_DEL -> {
                    session?.write(byteArrayOf(0x7f.toByte()))
                    true
                }
                KeyEvent.KEYCODE_DPAD_LEFT, KeyEvent.KEYCODE_DPAD_RIGHT,
                KeyEvent.KEYCODE_DPAD_UP, KeyEvent.KEYCODE_DPAD_DOWN -> true
                else -> {
                    val mods = ev.metaState and (KeyEvent.META_CTRL_ON
                        or KeyEvent.META_CTRL_LEFT_ON
                        or KeyEvent.META_CTRL_RIGHT_ON)
                    if (mods != 0) {
                        val ch = ev.unicodeChar and 0x1f
                        when (ch) {
                            3 -> { session?.sendSignal(2); true }
                            4 -> { session?.write(byteArrayOf(0x04)); true }
                            else -> if (ch != 0) { session?.write(byteArrayOf(ch.toByte())); true } else false
                        }
                    } else false
                }
            }
        }

        // ===== TextWatcher: catches '\n' insertion from KEYBOARD Enter =====
        // SKIPS dispatch when:
        //   - isPasteOperation = true  (paste in progress, multi-line content)
        //   - isDispatching = true     (we're inside dispatchEnter modifying text)
        //
        // When dispatching IS appropriate (keyboard Enter inserted '\n'):
        //   - Strip the '\n'
        //   - Call dispatchEnter() which takes the first line as cmd
        inputView.addTextChangedListener(object : TextWatcher {
            override fun beforeTextChanged(s: CharSequence?, start: Int, count: Int, after: Int) {}
            override fun onTextChanged(s: CharSequence?, start: Int, before: Int, count: Int) {}
            override fun afterTextChanged(s: Editable?) {
                if (s == null) return
                // CRITICAL: skip if paste is in progress
                if (inputView.isPasteOperation) return
                // Skip if we're inside dispatchEnter (modifying EditText)
                if (isDispatching) return
                val text = s.toString()
                val newlineIdx = text.indexOf('\n')
                if (newlineIdx < 0) return
                // Keyboard Enter detected — strip the '\n' and dispatch.
                // dispatchEnter will take the first line as cmd and keep the
                // rest in the EditText (if multi-line content).
                val cmd = text.substring(0, newlineIdx)
                if (cmd != text) {
                    // Replace the EditText content with text up to (but not
                    // including) the first '\n'. This strips the newline.
                    // If there's content after the newline, it stays.
                    val rest = text.substring(newlineIdx + 1)
                    val newText = if (rest.isNotEmpty()) cmd + rest else cmd
                    if (newText != text) {
                        s.replace(0, s.length, newText)
                    }
                }
                // s.replace triggers afterTextChanged again with text that may
                // still have '\n' if there was content after the first one.
                // But isDispatching guard (set in dispatchEnter below) prevents
                // recursive dispatch.
                dispatchEnter()
            }
        })
    }

    fun attach(s: TerminalSession) {
        session = s
        s.attachView(this)
    }

    fun detach() {
        session?.detachView(this)
        session = null
    }

    fun setCommandHandler(h: (String) -> Unit) {
        commandHandler = h
    }

    fun renderBuffer(buf: SpannableStringBuilder) {
        outputView.text = buf
    }

    fun requestScrollToBottom() {
        scrollView.post { scrollView.fullScroll(ScrollView.FOCUS_DOWN) }
    }

    fun requestInputFocus() {
        inputView.requestFocus()
    }

    /**
     * Takes the FIRST line of the EditText text as `cmd` (text before the
     * first '\n' if multi-line, or the whole text if single-line).
     * Updates the EditText to keep ONLY the remaining text (after the first
     * '\n') so subsequent Enters execute subsequent lines.
     * Clears if there's no remaining text.
     *
     * Then manually echoes `cmd\r\n` to the session buffer (PTY ECHO is off),
     * and delegates to commandHandler.
     *
     * GUARANTEES:
     *   - exactly ONE write to PTY per Enter (inside commandHandler)
     *   - multi-line paste content is preserved across multiple Enters
     *   - EditText is updated BEFORE session calls (no accumulation)
     */
    private fun dispatchEnter() {
        if (isDispatching) return
        isDispatching = true
        uiHandler.post { isDispatching = false }
        try {
            val text = inputView.text.toString()
            val newlineIdx = text.indexOf('\n')
            val cmd: String
            val rest: String
            if (newlineIdx >= 0) {
                cmd = text.substring(0, newlineIdx)
                rest = text.substring(newlineIdx + 1)
            } else {
                cmd = text
                rest = ""
            }
            // CRITICAL: update EditText BEFORE any session call so the next
            // Enter starts from the correct remaining text (or empty if single-line).
            // Setting isDispatching=true above prevents TextWatcher from
            // re-entering dispatchEnter during this modification.
            inputView.text.clear()
            if (rest.isNotEmpty()) {
                inputView.text.insert(0, rest)
            }
            // Manual echo — PTY ECHO is disabled in termios, so the user's
            // typed command appears in the buffer exactly once via this echo.
            session?.appendOutput(cmd + "\r\n")
            // Delegate: builtins → cyber shell; others → PTY (single write).
            commandHandler?.invoke(cmd)
        } catch (e: Exception) {
            android.util.Log.e("TerminalView", "dispatchEnter failed", e)
        }
    }
}
