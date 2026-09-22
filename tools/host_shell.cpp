// cyber-engine :: host shell demo (boots engine, executes user commands interactively)
#include "engine/Engine.hpp"
#include "engine/shell/Shell.hpp"
#include "engine/capabilities/CapabilityManager.hpp"
#include "engine/diagnostics/Diagnostics.hpp"

#include <cstdio>
#include <string>
#include <iostream>
#include <sstream>
#include <thread>

int main(int argc, char** argv) {
    using namespace cyber;
    auto& e = Engine::instance();
    auto r = e.create(nullptr, nullptr); // headless
    if (!r.ok()) {
        std::fprintf(stderr, "engine boot failed\n");
        return 1;
    }

    ShellContext ctx;
    ctx.getVersion = []() -> std::string {
        std::string out;
        Shell s(ShellContext{});
        s.execute("version", out);
        return out;
    };
    ctx.getCapabilities = []() -> std::string {
        std::ostringstream os;
        for (const auto& c : CapabilityManager::instance().all()) {
            os << capabilityName(c.cap) << "  " << capabilityStateLabel(c.state) << "  "
               << (c.note.empty() ? "-" : c.note) << "\n";
        }
        return os.str();
    };
    ctx.getDiagnostics = []() -> std::string {
        std::ostringstream os;
        std::array<DiagEntry, Diagnostics::kCapacity> buf{};
        Span<DiagEntry> view(buf.data(), buf.size());
        auto snap = Diagnostics::instance().snapshot(view);
        for (size_t i = 0; i < snap.size(); ++i) {
            const auto& en = snap[i];
            const char* lvl = "?";
            switch (en.level) {
                case DiagLevel::Info:  lvl = "INFO"; break;
                case DiagLevel::Warn:  lvl = "WARN"; break;
                case DiagLevel::Error: lvl = "ERROR"; break;
                case DiagLevel::Fatal: lvl = "FATAL"; break;
            }
            os << "[" << en.monotonicMs << "] " << lvl << " " << en.tag << ": "
               << en.message << "\n";
        }
        return os.str();
    };

    Shell shell(std::move(ctx));

    if (argc > 1) {
        // One-shot: join args and run
        std::string line;
        for (int i = 1; i < argc; ++i) {
            if (i > 1) line += ' ';
            line += argv[i];
        }
        std::string out;
        auto s = shell.execute(line, out);
        std::printf("%s", out.c_str());
        return s == Status::Ok ? 0 : 1;
    }

    // Interactive
    std::printf("cyber-engine shell — type 'help'\n");
    std::string line;
    while (std::printf("cyber> "), std::getline(std::cin, line)) {
        std::string out;
        auto s = shell.execute(line, out);
        std::printf("%s", out.c_str());
        (void)s;
    }
    e.destroy();
    return 0;
}
