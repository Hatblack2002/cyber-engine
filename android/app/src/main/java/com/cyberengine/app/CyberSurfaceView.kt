// Cyber Engine :: CyberSurfaceView
// Thin SurfaceView wrapper that exposes the holder.
// MUST declare (Context, AttributeSet) constructor for XML inflation.
package com.cyberengine.app

import android.content.Context
import android.util.AttributeSet
import android.view.SurfaceHolder
import android.view.SurfaceView

class CyberSurfaceView @JvmOverloads constructor(
    ctx: Context,
    attrs: AttributeSet? = null,
    defStyle: Int = 0
) : SurfaceView(ctx, attrs, defStyle) {

    init {
        setZOrderOnTop(true)
        holder.setFormat(android.graphics.PixelFormat.OPAQUE)
    }
}
