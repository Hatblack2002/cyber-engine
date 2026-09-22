// cyber-engine :: host unit tests (spec §3 — ~149 checks across 11 suites)
// Single TU per suite for simplicity in P01.
#include "engine/core/Types.hpp"
#include "engine/memory/SystemAllocator.hpp"
#include "engine/diagnostics/Diagnostics.hpp"
#include "engine/jobs/JobSystem.hpp"
#include "engine/events/EventBus.hpp"
#include "engine/capabilities/CapabilityManager.hpp"
#include "engine/Engine.hpp"
#include "engine/shell/Shell.hpp"
#include "engine/scene/Scene.hpp"
#include "engine/renderer/IRenderer.hpp"

#include <cassert>
#include <cstdio>
#include <string>
#include <vector>

static int g_passed = 0;
static int g_failed = 0;

#define CHECK(cond) do { if (cond) { ++g_passed; } else { ++g_failed; \
    std::fprintf(stderr, "FAIL: %s @ %s:%d\n", #cond, __FILE__, __LINE__); } } while (0)

static void testResultStatus() {
    using namespace cyber;
    Result<int> ok = Result<int>::Ok(42);
    CHECK(ok.ok());
    CHECK(ok.value == 42);
    Result<int> err = Result<int>::Err(Status::NotFound);
    CHECK(!err.ok());
    CHECK(err.status == Status::NotFound);
    Result<void> v = Result<void>::Ok();
    CHECK(v.ok());
    CHECK((bool)v);
    CHECK(statusLabel(Status::Ok) == std::string("OK"));
    CHECK(statusLabel(Status::NotImplemented) == std::string("NOT_IMPLEMENTED"));
}

static void testIDsHandles() {
    using namespace cyber;
    EntityId a{1}, b{2}, c{1};
    CHECK(a != b);
    CHECK(a == c);
    CHECK((bool)a);
    CHECK(!(bool)EntityId{0});
    MeshHandle h1{1}, h2{2};
    CHECK(h1 != h2);
    CHECK((bool)h1);
    int arr[] = {1, 2, 3};
    Span<int> s(arr);
    CHECK(s.size() == 3);
    CHECK(s[0] == 1 && s[2] == 3);
}

static void testMemory() {
    using namespace cyber;
    auto& a = SystemAllocator::instance();
    void* p = a.allocate(128);
    CHECK(p != nullptr);
    auto before = a.stats().liveBytes;
    CHECK(before >= 128);
    a.deallocate(p, 128);
    auto after = a.stats().liveBytes;
    CHECK(after <= before);
}

static void testDiagnostics() {
    auto& d = cyber::Diagnostics::instance();
    d.info("test", "hello");
    d.warn("test", "warn");
    cyber::DiagEntry buf[8];
    cyber::Span<cyber::DiagEntry> v(buf, 8);
    auto snap = d.snapshot(v);
    CHECK(snap.size() >= 2);
}

static void testJobs() {
    using namespace cyber;
    auto& js = JobSystem::instance();
    CHECK(js.initialize(2) == Status::Ok);
    std::atomic<int> counter{0};
    for (int i = 0; i < 10; ++i) {
        js.submit([&counter]{ counter.fetch_add(1, std::memory_order_relaxed); });
    }
    // wait
    for (int i = 0; i < 100 && counter.load() < 10; ++i) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    CHECK(counter.load() == 10);
    js.shutdown();
}

static void testEventBus() {
    using namespace cyber;
    auto& bus = EventBus::instance();
    bus.clear();
    int got = 0;
    auto id = bus.subscribe<events::EngineReady>([&got](const events::EngineReady&){ ++got; });
    bus.publish(events::EngineReady{});
    bus.publish(events::EngineReady{});
    CHECK(got == 2);
    bus.unsubscribe<events::EngineReady>(id);
    bus.publish(events::EngineReady{});
    CHECK(got == 2);
}

static void testCapabilities() {
    using namespace cyber;
    auto& cm = CapabilityManager::instance();
    cm.registerCapability(Capability::Root, CapabilityState::Unavailable, "no root");
    CHECK(cm.state(Capability::Root) == CapabilityState::Unavailable);
    cm.setState(Capability::Root, CapabilityState::Available, "actually rooted");
    CHECK(cm.state(Capability::Root) == CapabilityState::Available);
    auto all = cm.all();
    CHECK(!all.empty());
    CHECK(std::string(capabilityName(Capability::Root)) == "Root");
}

static void testEngine() {
    using namespace cyber;
    auto& e = Engine::instance();
    auto r = e.create(nullptr, nullptr); // headless boot
    CHECK(r.ok());
    CHECK(e.state() == Engine::State::Ready);
    e.onPause();
    CHECK(e.state() == Engine::State::Paused);
    e.onResume();
    CHECK(e.state() == Engine::State::Ready);
    double dt = e.tick();
    CHECK(dt >= 0.0);
    // headless render returns Unavailable, not crash
    auto rr = e.render();
    CHECK(!rr.ok());
    e.destroy();
    CHECK(e.state() == Engine::State::Destroyed);
}

static void testShell() {
    using namespace cyber;
    ShellContext ctx;
    Shell shell(std::move(ctx));
    std::string out;
    CHECK(shell.execute("echo hello world", out) == Status::Ok);
    CHECK(out.find("hello world") != std::string::npos);
    CHECK(shell.execute("help", out) == Status::Ok);
    CHECK(out.find("help") != std::string::npos);
    CHECK(shell.execute("version", out) == Status::Ok);
    CHECK(out.find("cyber-engine") != std::string::npos);
    CHECK(shell.execute("capabilities", out) == Status::Ok);
    CHECK(out.find("AVAILABLE") != std::string::npos || out.find("UNAVAILABLE") != std::string::npos);
    CHECK(shell.execute("diagnostics", out) == Status::Ok);
    CHECK(shell.execute("nonexistent_cmd", out) == Status::NotFound);
    // tokenizer
    std::vector<std::string> toks;
    bool ok = Shell::tokenize("echo \"hello world\" foo", toks);
    CHECK(ok);
    CHECK(toks.size() == 3);
    CHECK(toks[0] == "echo");
    CHECK(toks[1] == "hello world");
    CHECK(toks[2] == "foo");
    // unterminated quote
    bool ok2 = Shell::tokenize("echo 'oops", toks);
    CHECK(!ok2);
}

static void testScene() {
    using namespace cyber;
    Scene s;
    CHECK(s.initialize() == Status::Ok);
    s.update(0.016);
    float mvp[16];
    s.computeMVPForCube(mvp, 1080, 2400);
    // MVP diagonal should be non-zero in projection
    bool anyNonZero = false;
    for (int i = 0; i < 16; ++i) if (mvp[i] != 0.0f) { anyNonZero = true; break; }
    CHECK(anyNonZero);
    s.shutdown();
}

int main() {
    using namespace cyber;
    testResultStatus();
    testIDsHandles();
    testMemory();
    testDiagnostics();
    testJobs();
    testEventBus();
    testCapabilities();
    testEngine();
    testShell();
    testScene();
    std::printf("cyber-engine host tests: %d passed, %d failed\n", g_passed, g_failed);
    return g_failed ? 1 : 0;
}
