// Cyber Engine :: CyberEditText
// Custom EditText that distinguishes real keyboard Enter from PASTE operations.
//
// The problem: a multi-line paste inserts '\n' chars into the EditText, which
// the TextWatcher in TerminalView would interpret as a keyboard Enter and
// auto-dispatch the first line, losing the rest of the pasted content.
//
// Solution: override onTextContextMenuItem to set a flag isPasteOperation=true
// while a paste is in progress. The TextWatcher checks this flag and skips
// dispatch when the modification comes from a paste.
//
// The flag is cleared via post() so any queued TextWatcher callbacks (some
// IMEs process InputConnection asynchronously) still see the flag.
package com.cyberengine.app

import android.content.Context
import android.util.AttributeSet
import android.widget.EditText

class CyberEditText @JvmOverloads constructor(
    ctx: Context,
    attrs: AttributeSet? = null,
    defStyle: Int = android.R.attr.editTextStyle
) : EditText(ctx, attrs, defStyle) {

    @Volatile
    var isPasteOperation: Boolean = false
        private set

    override fun onTextContextMenuItem(id: Int): Boolean {
        if (id == android.R.id.paste || id == android.R.id.pasteAsPlainText
            || id == android.R.id.copy || id == android.R.id.cut) {
            if (id == android.R.id.paste || id == android.R.id.pasteAsPlainText) {
                isPasteOperation = true
                // Clear AFTER the current call stack unwinds. This ensures any
                // TextWatcher callbacks (sync or queued) see isPasteOperation=true.
                post { isPasteOperation = false }
            }
        }
        return super.onTextContextMenuItem(id)
    }
}
