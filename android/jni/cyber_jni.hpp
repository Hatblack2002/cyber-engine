// Cyber Engine :: JNI bridge (entry points called from Kotlin)
// All functions take a `jlong handle` that is the Engine* pointer (casted).
#pragma once
#include <cstdint>
#include <jni.h>

namespace cyber {
class Engine;
class Shell;
} // namespace cyber

namespace cyber_jni {

// Engine handle <-> C++ pointer conversion
inline cyber::Engine* engineFromHandle(jlong h) noexcept {
    return reinterpret_cast<cyber::Engine*>(static_cast<uintptr_t>(h));
}
inline jlong handleFromEngine(cyber::Engine* e) noexcept {
    return static_cast<jlong>(reinterpret_cast<uintptr_t>(e));
}

} // namespace cyber_jni
