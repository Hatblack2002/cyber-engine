// cyber-engine :: scene implementation
#include "engine/scene/Scene.hpp"
#include "engine/diagnostics/Diagnostics.hpp"
#include "engine/capabilities/CapabilityManager.hpp"

namespace cyber {

Status Scene::initialize() noexcept {
    camera_.target = Vec3{0, 0, 0};
    camera_.distance = 5.0f;
    camera_.yaw = 0.7f;
    camera_.pitch = 0.4f;
    cubeAngle_ = 0.0f;
    elapsed_ = 0.0;
    CapabilityManager::instance().setState(
        Capability::Scene3D, CapabilityState::Available,
        "Cube mesh + orbit camera + perspective");
    CapabilityManager::instance().setState(
        Capability::Camera, CapabilityState::Available,
        "Orbit / pan / zoom camera");
    CapabilityManager::instance().setState(
        Capability::Touch, CapabilityState::Available,
        "Touch -> orbit (single), zoom (pinch)");
    CapabilityManager::instance().setState(
        Capability::Picking, CapabilityState::NotImplemented,
        "NDC ray intersection pending — spec §9");
    return Status::Ok;
}

Status Scene::shutdown() noexcept {
    return Status::Ok;
}

void Scene::update(double deltaSec) noexcept {
    elapsed_ += deltaSec;
    cubeAngle_ += static_cast<float>(deltaSec * 0.6);
    if (cubeAngle_ > 6.2831853f) cubeAngle_ -= 6.2831853f;
}

void Scene::computeMVPForCube(float out16[16], int vw, int vh) const noexcept {
    float aspect = (vh > 0 && vw > 0) ? static_cast<float>(vw) / static_cast<float>(vh) : 1.0f;
    Mat4 proj = Mat4::perspective(0.785f, aspect, 0.1f, 100.0f); // 45deg
    Mat4 view = camera_.view();
    Mat4 model = Mat4::multiply(Mat4::rotationY(cubeAngle_), Mat4::rotationY(cubeAngle_ * 0.5f));
    Mat4 mv = Mat4::multiply(view, model);
    Mat4 mvp = Mat4::multiply(proj, mv);
    for (int i = 0; i < 16; ++i) out16[i] = mvp.m[i];
}

} // namespace cyber
