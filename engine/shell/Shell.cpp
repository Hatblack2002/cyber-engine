// cyber-engine :: shell implementation
#include "engine/shell/Shell.hpp"
#include "engine/diagnostics/Diagnostics.hpp"
#include "engine/capabilities/CapabilityManager.hpp"
#include <sstream>

namespace cyber {

// Static version string. Bumped per build; never hardcode "Vulkan OK".
constexpr const char* kCyberVersion = "cyber-engine v3 0.1.0 (cybercommand os / portrait / own-linux-bridge)";

bool Shell::tokenize(StrView line, std::vector<std::string>& out) noexcept {
    out.clear();
    std::string cur;
    char quote = 0;
    for (size_t i = 0; i < line.size(); ++i) {
        char c = line[i];
        if (quote) {
            if (c == quote) { quote = 0; continue; }
            cur.push_back(c);
            continue;
        }
        if (c == '"' || c == '\'') { quote = c; continue; }
        if (c == ' ' || c == '\t' || c == '\r' || c == '\n') {
            if (!cur.empty()) { out.push_back(std::move(cur)); cur.clear(); }
            continue;
        }
        cur.push_back(c);
    }
    if (quote) return false; // unterminated
    if (!cur.empty()) out.push_back(std::move(cur));
    return true;
}

void Shell::registerBuiltins() {
    commands_["help"] = [this](const std::vector<std::string>&, std::string& out) -> Status {
        std::ostringstream os;
        os << "Cyber Engine Shell — built-in commands:\n";
        os << "  help                  Show this help\n";
        os << "  version               Show engine version (real, not hardcoded)\n";
        os << "  capabilities          List capabilities and their honest states\n";
        os << "  diagnostics           Dump diagnostics ring buffer\n";
        os << "  echo [text]           Echo arguments (respects quotes)\n";
        os << "  clear                 Clear terminal (signal to UI)\n";
        out = os.str();
        return Status::Ok;
    };
    commands_["version"] = [this](const std::vector<std::string>&, std::string& out) -> Status {
        out = std::string(kCyberVersion) + "\n";
        out += "  renderer:  ";
        // Pull real renderer state from CapabilityManager (spec §7: never hardcode "Vulkan OK")
        auto vk = CapabilityManager::instance().info(Capability::Renderer_Vulkan);
        auto gl = CapabilityManager::instance().info(Capability::Renderer_GLES);
        out += "Vulkan=";
        out += capabilityStateLabel(vk.state);
        out += "  GLES=";
        out += capabilityStateLabel(gl.state);
        out += "\n";
        return Status::Ok;
    };
    commands_["capabilities"] = [this](const std::vector<std::string>&, std::string& out) -> Status {
        if (ctx_.getCapabilities) { out = ctx_.getCapabilities(); return Status::Ok; }
        std::ostringstream os;
        os << "Capability          State                Note\n";
        os << "----------------------------------------------\n";
        for (const auto& c : CapabilityManager::instance().all()) {
            os << capabilityName(c.cap);
            for (int i = (int)strlen(capabilityName(c.cap)); i < 20; ++i) os << ' ';
            os << capabilityStateLabel(c.state);
            for (int i = (int)strlen(capabilityStateLabel(c.state)); i < 22; ++i) os << ' ';
            os << (c.note.empty() ? "-" : c.note) << '\n';
        }
        out = os.str();
        return Status::Ok;
    };
    commands_["diagnostics"] = [this](const std::vector<std::string>&, std::string& out) -> Status {
        if (ctx_.getDiagnostics) { out = ctx_.getDiagnostics(); return Status::Ok; }
        std::ostringstream os;
        os << "Diagnostics ring (last N entries):\n";
        std::array<DiagEntry, Diagnostics::kCapacity> buf{};
        Span<DiagEntry> view(buf.data(), buf.size());
        auto snap = Diagnostics::instance().snapshot(view);
        for (size_t i = 0; i < snap.size(); ++i) {
            const auto& e = snap[i];
            const char* lvl = "?";
            switch (e.level) {
                case DiagLevel::Info:  lvl = "INFO";  break;
                case DiagLevel::Warn:  lvl = "WARN";  break;
                case DiagLevel::Error: lvl = "ERROR"; break;
                case DiagLevel::Fatal: lvl = "FATAL"; break;
            }
            os << "[" << e.monotonicMs << "] " << lvl << " " << e.tag << ": " << e.message << "\n";
        }
        out = os.str();
        return Status::Ok;
    };
    commands_["echo"] = [this](const std::vector<std::string>& args, std::string& out) -> Status {
        std::ostringstream os;
        for (size_t i = 1; i < args.size(); ++i) {
            if (i > 1) os << ' ';
            os << args[i];
        }
        os << '\n';
        out = os.str();
        return Status::Ok;
    };
    commands_["clear"] = [this](const std::vector<std::string>&, std::string&) -> Status {
        // UI watches for this magic token to clear the screen buffer.
        // Not an output string — signal via EventBus would be cleaner; for P01
        // we use a sentinel the UI recognizes.
        return Status::Ok;
    };
}

bool Shell::registerCommand(std::string name,
        std::function<Status(const std::vector<std::string>&, std::string&)> fn) noexcept {
    if (commands_.count(name)) return false;
    commands_[name] = std::move(fn);
    return true;
}

Status Shell::execute(StrView line, std::string& out) noexcept {
    std::vector<std::string> args;
    if (!tokenize(line, args)) {
        out = "shell: parse error: unterminated quote\n";
        return Status::InvalidArgument;
    }
    if (args.empty()) return Status::Ok;
    auto it = commands_.find(args[0]);
    if (it == commands_.end()) {
        out = "shell: unknown command: " + args[0] + " (try 'help')\n";
        return Status::NotFound;
    }
    try {
        return it->second(args, out);
    } catch (const std::exception& e) {
        out = std::string("shell: exception: ") + e.what() + "\n";
        return Status::Error;
    }
}

std::vector<std::string> Shell::commandNames() const {
    std::vector<std::string> v;
    v.reserve(commands_.size());
    for (const auto& [n, _] : commands_) v.push_back(n);
    std::sort(v.begin(), v.end());
    return v;
}

} // namespace cyber
