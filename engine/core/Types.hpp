// cyber-engine :: core types (spec §3 — Result, Status, IDs, Handles, Span, StringView)
// C++20, header-only, no exceptions, no RTTI required.
#pragma once
#include <cstdint>
#include <cstddef>
#include <cstring>
#include <utility>
#include <type_traits>
#include <string_view>

namespace cyber {

// ----- Status (spec §3): todas las operaciones retornan Status; nada de bool ciego -----
enum class Status : uint8_t {
    Ok = 0,
    Error,
    InvalidArgument,
    NotImplemented,
    Unsupported,
    Unavailable,
    RequiresPermission,
    RequiresRoot,
    RequiresHardware,
    NotFound,
    AlreadyExists,
    OutOfMemory,
    BadState,
};

inline const char* statusLabel(Status s) noexcept {
    switch (s) {
        case Status::Ok: return "OK";
        case Status::Error: return "ERROR";
        case Status::InvalidArgument: return "INVALID_ARGUMENT";
        case Status::NotImplemented: return "NOT_IMPLEMENTED";
        case Status::Unsupported: return "UNSUPPORTED";
        case Status::Unavailable: return "UNAVAILABLE";
        case Status::RequiresPermission: return "REQUIRES_PERMISSION";
        case Status::RequiresRoot: return "REQUIRES_ROOT";
        case Status::RequiresHardware: return "REQUIRES_HARDWARE";
        case Status::NotFound: return "NOT_FOUND";
        case Status::AlreadyExists: return "ALREADY_EXISTS";
        case Status::OutOfMemory: return "OUT_OF_MEMORY";
        case Status::BadState: return "BAD_STATE";
    }
    return "?";
}

// ----- Result<T>: Status + optional payload. Nunca lanza. -----
template <typename T = void>
struct Result {
    Status status = Status::Error;
    T value{};
    explicit constexpr operator bool() const noexcept { return status == Status::Ok; }
    constexpr bool ok() const noexcept { return status == Status::Ok; }
    static Result Ok(T v) noexcept { return Result{Status::Ok, std::move(v)}; }
    static Result Err(Status s) noexcept { return Result{s, T{}}; }
};

template <>
struct Result<void> {
    Status status = Status::Error;
    explicit constexpr operator bool() const noexcept { return status == Status::Ok; }
    constexpr bool ok() const noexcept { return status == Status::Ok; }
    static Result Ok() noexcept { return Result{Status::Ok}; }
    static Result Err(Status s) noexcept { return Result{s}; }
};

// ----- IDs: typed strong integers (no implicit cross-type conversion) -----
#define CYBER_STRONG_ID(Name, Underlying)                              \
    struct Name {                                                     \
        Underlying value{};                                           \
        constexpr Name() = default;                                   \
        explicit constexpr Name(Underlying v) noexcept : value(v) {} \
        constexpr bool operator==(Name o) const noexcept { return value == o.value; } \
        constexpr bool operator!=(Name o) const noexcept { return value != o.value; } \
        constexpr bool operator<(Name o) const noexcept { return value < o.value; }   \
        constexpr explicit operator bool() const noexcept { return value != 0; }      \
    }

CYBER_STRONG_ID(EntityId, uint64_t);
CYBER_STRONG_ID(CapabilityId, uint32_t);
CYBER_STRONG_ID(JobId, uint64_t);
CYBER_STRONG_ID(EventSubId, uint64_t);

#undef CYBER_STRONG_ID

// ----- Handle<T>: non-owning, nullable, typed wrapper -----
template <typename Tag>
struct Handle {
    uint64_t raw = 0;
    constexpr Handle() = default;
    explicit constexpr Handle(uint64_t v) noexcept : raw(v) {}
    constexpr bool operator==(Handle o) const noexcept { return raw == o.raw; }
    constexpr bool operator!=(Handle o) const noexcept { return raw != o.raw; }
    constexpr explicit operator bool() const noexcept { return raw != 0; }
};

struct RendererHandleTag {};
struct MeshHandleTag {};
struct ShaderHandleTag {};
using RendererHandle = Handle<RendererHandleTag>;
using MeshHandle = Handle<MeshHandleTag>;
using ShaderHandle = Handle<ShaderHandleTag>;

// ----- Span<T>: non-owning view (no std::vector alloc) -----
template <typename T>
struct Span {
    T* data = nullptr;
    size_t n = 0;
    constexpr Span() = default;
    constexpr Span(T* d, size_t c) noexcept : data(d), n(c) {}
    template <size_t N>
    constexpr Span(T (&arr)[N]) noexcept : data(arr), n(N) {}
    constexpr T& operator[](size_t i) noexcept { return data[i]; }
    constexpr const T& operator[](size_t i) const noexcept { return data[i]; }
    constexpr size_t size() const noexcept { return n; }
    constexpr bool empty() const noexcept { return n == 0; }
    constexpr T* begin() noexcept { return data; }
    constexpr T* end() noexcept { return data + n; }
    constexpr const T* begin() const noexcept { return data; }
    constexpr const T* end() const noexcept { return data + n; }
};

// ----- StringView: convenience alias -----
using StrView = std::string_view;

} // namespace cyber
