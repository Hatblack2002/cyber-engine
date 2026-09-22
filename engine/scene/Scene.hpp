// cyber-engine :: scene (spec §9 — Camera, Mesh, Light; minimal but real)
#pragma once
#include "engine/core/Types.hpp"
#include "engine/memory/SystemAllocator.hpp"
#include <cstdint>
#include <cmath>

namespace cyber {

// Minimal math — no GLM dependency. Column-major, OpenGL-style.
// 4x4 matrix stored as 16 contiguous floats; index = col*4 + row.
struct Mat4 {
    float m[16]{};
    static Mat4 identity() noexcept {
        Mat4 r;
        for (int i = 0; i < 16; ++i) r.m[i] = 0.0f;
        r.m[0] = r.m[5] = r.m[10] = r.m[15] = 1.0f;
        return r;
    }
    static Mat4 perspective(float fovyRad, float aspect, float zn, float zf) noexcept {
        Mat4 r;
        float f = 1.0f / std::tan(fovyRad * 0.5f);
        for (float& x : r.m) x = 0.0f;
        r.m[0] = f / aspect;
        r.m[5] = f;
        r.m[10] = (zf + zn) / (zn - zf);
        r.m[11] = -1.0f;
        r.m[14] = (2.0f * zf * zn) / (zn - zf);
        return r;
    }
    static Mat4 translation(float x, float y, float z) noexcept {
        Mat4 r = identity();
        r.m[12] = x; r.m[13] = y; r.m[14] = z;
        return r;
    }
    static Mat4 rotationY(float rad) noexcept {
        Mat4 r = identity();
        float c = std::cos(rad), s = std::sin(rad);
        r.m[0] = c;  r.m[2] = s;
        r.m[8] = -s; r.m[10] = c;
        return r;
    }
    static Mat4 lookAt(float ex, float ey, float ez,
                       float tx, float ty, float tz,
                       float ux, float uy, float uz) noexcept {
        float fx = tx-ex, fy = ty-ey, fz = tz-ez;
        float rl = 1.0f / std::sqrt(fx*fx + fy*fy + fz*fz);
        fx*=rl; fy*=rl; fz*=rl;
        // right = f × up
        float rx = fy*uz - fz*uy;
        float ry = fz*ux - fx*uz;
        float rz = fx*uy - fy*ux;
        float rr = 1.0f / std::sqrt(rx*rx + ry*ry + rz*rz);
        rx*=rr; ry*=rr; rz*=rr;
        // up' = right × f
        float Ux = ry*fz - rz*fy;
        float Uy = rz*fx - rx*fz;
        float Uz = rx*fy - ry*fx;
        Mat4 r = identity();
        r.m[0]=rx; r.m[4]=ry; r.m[8]=rz;
        r.m[1]=Ux; r.m[5]=Uy; r.m[9]=Uz;
        r.m[2]=-fx; r.m[6]=-fy; r.m[10]=-fz;
        r.m[12] = -(rx*ex + ry*ey + rz*ez);
        r.m[13] = -(Ux*ex + Uy*ey + Uz*ez);
        r.m[14] = -(-fx*ex - fy*ey - fz*ez);
        return r;
    }
    static Mat4 multiply(const Mat4& a, const Mat4& b) noexcept {
        Mat4 r;
        for (int c = 0; c < 4; ++c) {
            for (int row = 0; row < 4; ++row) {
                float s = 0.0f;
                for (int k = 0; k < 4; ++k) {
                    s += a.m[k*4 + row] * b.m[c*4 + k];
                }
                r.m[c*4 + row] = s;
            }
        }
        return r;
    }
};

struct Vec3 { float x = 0, y = 0, z = 0; };

class Camera {
public:
    Vec3 target{0, 0, 0};
    float distance = 5.0f;
    float yaw   = 0.0f;
    float pitch = 0.4f;

    void orbit(float dYaw, float dPitch) noexcept {
        yaw += dYaw;
        pitch += dPitch;
        if (pitch > 1.5f) pitch = 1.5f;
        if (pitch < -1.5f) pitch = -1.5f;
    }
    void pan(float dx, float dy) noexcept {
        // pan target on camera's right/up basis
        target.x += dx; target.y += dy;
    }
    void zoom(float factor) noexcept {
        distance *= factor;
        if (distance < 1.5f) distance = 1.5f;
        if (distance > 30.0f) distance = 30.0f;
    }
    Vec3 eye() const noexcept {
        float cp = std::cos(pitch), sp = std::sin(pitch);
        float cy = std::cos(yaw),   sy = std::sin(yaw);
        return Vec3{
            target.x + distance * cp * sy,
            target.y + distance * sp,
            target.z + distance * cp * cy
        };
    }
    Mat4 view() const noexcept {
        Vec3 e = eye();
        return Mat4::lookAt(e.x, e.y, e.z,
                            target.x, target.y, target.z,
                            0, 1, 0);
    }
};

class Scene {
public:
    Status initialize() noexcept;
    Status shutdown() noexcept;
    void   update(double deltaSec) noexcept;
    void   orbitCamera(float dYaw, float dPitch) noexcept { camera_.orbit(dYaw, dPitch); }
    void   panCamera(float dx, float dy) noexcept { camera_.pan(dx, dy); }
    void   zoomCamera(float f) noexcept { camera_.zoom(f); }
    const Camera& camera() const noexcept { return camera_; }

    // Computes MVP for the cube mesh (used by GLESRenderer).
    void computeMVPForCube(float out16[16], int viewportW, int viewportH) const noexcept;

private:
    Camera camera_;
    float  cubeAngle_ = 0.0f;
    double elapsed_ = 0.0;
};

} // namespace cyber
