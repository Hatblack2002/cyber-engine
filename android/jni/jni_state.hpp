// Cyber Engine :: JNI state holders (renderer + scene singletons)
// Provides getters used by cyber_jni.cpp for surface/render/touch callbacks.
#pragma once
#include "engine/renderer/IRenderer.hpp"
#include "engine/scene/Scene.hpp"

using namespace cyber;

namespace cyber_jni {
// Owned by JNI layer; created in nativeEngineCreate, destroyed in destroy.
inline IRenderer*& rendererSlot() {
    static IRenderer* r = nullptr;
    return r;
}
inline Scene*& sceneSlot() {
    static Scene* s = nullptr;
    return s;
}
} // namespace cyber_jni

inline cyber::IRenderer* cyber_jni_getRenderer() { return cyber_jni::rendererSlot(); }
inline cyber::Scene*     cyber_jni_getScene()    { return cyber_jni::sceneSlot(); }
