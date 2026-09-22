// cyber-engine :: shell (spec §3, §7)
// Commands P01: help, version, capabilities, diagnostics, echo, clear.
// Parser: tokens + single/double quotes. NO pipes, NO && until implemented.
#pragma once
#include "engine/core/Types.hpp"
#include <functional>
#include <string>
#include <string_view>
#include <vector>
#include <unordered_map>

namespace cyber {

struct ShellContext {
    std::function<std::string()> getVersion;     // returns version string (e.g., "cyber-engine v3 0.1.0")
    std::function<std::string()> getDiagnostics; // returns formatted diagnostics dump
    std::function<std::string()> getCapabilities; // returns formatted capabilities dump
};

class Shell {
public:
    explicit Shell(ShellContext ctx) : ctx_(std::move(ctx)) {
        registerBuiltins();
    }

    // Tokenize a raw command line, honoring single and double quotes.
    // Returns false on unterminated quote. Does NOT honor pipes/&& (spec §7).
    static bool tokenize(StrView line, std::vector<std::string>& out) noexcept;

    // Execute a single command line. Output (stdout+stderr) is appended to out.
    // Returns Status::Ok on success, Status::NotFound if the command is unknown.
    Status execute(StrView line, std::string& out) noexcept;

    // Register an additional command (host or runtime). Returns false if name collides.
    bool registerCommand(std::string name,
                         std::function<Status(const std::vector<std::string>&, std::string&)> fn) noexcept;

    std::vector<std::string> commandNames() const;

private:
    void registerBuiltins();
    ShellContext ctx_;
    std::unordered_map<std::string,
        std::function<Status(const std::vector<std::string>&, std::string&)>> commands_;
};

} // namespace cyber
