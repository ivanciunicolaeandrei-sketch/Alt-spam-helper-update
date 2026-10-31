#include <Geode/Geode.hpp>
#include <Geode/loader/SettingV3.hpp>
#include <algorithm>
#include <fstream>
#include <regex>
#include <string>
#include <vector>

using namespace geode::prelude;

namespace {
    bool s_altSpamDetected = false;

    void analyseMacro(std::filesystem::path const& path) {
        s_altSpamDetected = false;

        if (path.empty()) {
            log::info("Alt Spam Helper: no macro selected.");
            return;
        }

        std::ifstream input(path, std::ios::binary);
        if (!input) {
            log::warn("Alt Spam Helper: could not open selected macro: {}", path.string());
            return;
        }

        // Read a bounded amount so a malformed/huge file cannot consume
        // unlimited memory during analysis.
        constexpr size_t MAX_MACRO_BYTES = 16 * 1024 * 1024;
        std::string data;
        data.resize(MAX_MACRO_BYTES);
        input.read(data.data(), static_cast<std::streamsize>(data.size()));
        data.resize(static_cast<size_t>(input.gcount()));

        // Prototype support: JSON-like event objects using "button" or "btn"
        // with values 1/2. Binary formats such as .gdr are not decoded yet.
        static const std::regex buttonField(
            R"("(?:button|btn)"\s*:\s*([12]))",
            std::regex_constants::icase
        );

        std::vector<int> buttons;
        for (std::sregex_iterator it(data.begin(), data.end(), buttonField), end;
             it != end; ++it) {
            buttons.push_back(std::stoi((*it)[1].str()));
        }

        size_t longestAlternation = 0;
        size_t currentAlternation = 0;
        for (size_t i = 1; i < buttons.size(); ++i) {
            if (buttons[i] != buttons[i - 1]) {
                ++currentAlternation;
                longestAlternation = std::max(longestAlternation, currentAlternation);
            } else {
                currentAlternation = 0;
            }
        }

        // Five consecutive switches means at least six alternating events.
        s_altSpamDetected = longestAlternation >= 5;

        if (s_altSpamDetected) {
            log::info(
                "Alt Spam Helper: possible alternating input pattern found ({} button events).",
                buttons.size()
            );
        } else if (buttons.empty()) {
            log::warn(
                "Alt Spam Helper: no supported JSON button events found. "
                "This does not mean the macro contains no alternating inputs."
            );
        } else {
            log::info(
                "Alt Spam Helper: no sustained alternating pattern found ({} button events).",
                buttons.size()
            );
        }
    }
}

$on_mod(Loaded) {
    auto selected = Mod::get()->getSettingValue<std::filesystem::path>("macro_file");
    analyseMacro(selected);

    listenForSettingChanges("macro_file", [](std::filesystem::path const& path) {
        analyseMacro(path);
    });

    log::info("Alt Spam Helper prototype loaded.");
}
