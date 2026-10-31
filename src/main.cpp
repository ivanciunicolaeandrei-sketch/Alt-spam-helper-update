#include <Geode/Geode.hpp>
#include <Geode/loader/SettingV3.hpp>
#include <gdr/gdr.hpp>
#include <algorithm>
#include <fstream>
#include <regex>
#include <string>
#include <vector>

using namespace geode::prelude;

namespace {
    struct HelperInput : gdr::Input {
        HelperInput() : gdr::Input(0, 1, false, false) {}
    };

    struct HelperReplay : gdr::Replay<HelperReplay, HelperInput> {
        HelperReplay() : Replay("Alt Spam Helper", "0.1.2") {}
    };

    bool s_altSpamDetected = false;

    bool detectAlternation(std::vector<int> const& buttons) {
        size_t longest = 0;
        size_t current = 0;
        for (size_t i = 1; i < buttons.size(); ++i) {
            if (buttons[i] != buttons[i - 1]) {
                ++current;
                longest = std::max(longest, current);
            } else {
                current = 0;
            }
        }
        return longest >= 5;
    }

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

        input.seekg(0, std::ios::end);
        auto length = input.tellg();
        if (length <= 0 || length > static_cast<std::streamoff>(16 * 1024 * 1024)) {
            log::warn("Alt Spam Helper: macro is empty or larger than 16 MB.");
            return;
        }
        input.seekg(0, std::ios::beg);
        std::vector<uint8_t> bytes(static_cast<size_t>(length));
        input.read(reinterpret_cast<char*>(bytes.data()), static_cast<std::streamsize>(bytes.size()));
        if (!input) {
            log::warn("Alt Spam Helper: failed to read the complete macro.");
            return;
        }

        // GDR1 .gdr files are MessagePack; .gdr.json is JSON. The
        // GDReplayFormat importer detects MessagePack first, then JSON.
        if (bytes.size() >= 3 && bytes[0] == 'G' && bytes[1] == 'D' && bytes[2] == 'R') {
            log::warn("Alt Spam Helper: GDR2 header detected. This build supports legacy GDR1 only.");
            return;
        }

        try {
            auto replay = HelperReplay::importData(bytes, true);
            std::vector<int> pressed;
            pressed.reserve(replay.inputs.size());
            for (auto const& event : replay.inputs) {
                if (event.down && !event.player2 && event.button >= 1 && event.button <= 3) {
                    pressed.push_back(event.button);
                }
            }

            s_altSpamDetected = detectAlternation(pressed);
            log::info(
                "Alt Spam Helper: imported GDR1 replay ({} inputs, {:.1f} FPS). {}",
                replay.inputs.size(),
                replay.framerate,
                s_altSpamDetected ? "Possible alternating-input pattern found."
                                  : "No sustained alternating-input pattern found."
            );
            return;
        } catch (std::exception const& error) {
            log::warn("Alt Spam Helper: GDR import failed: {}", error.what());
        } catch (...) {
            log::warn("Alt Spam Helper: GDR import failed with an unknown error.");
        }

        // Keep the earlier lightweight JSON-style detector as a fallback
        // for non-GDR macro files.
        std::string data(bytes.begin(), bytes.end());
        static const std::regex buttonField(
            R"REGEX("(?:button|btn)"\\s*:\\s*([12]))REGEX",
            std::regex_constants::icase
        );
        std::vector<int> buttons;
        for (std::sregex_iterator it(data.begin(), data.end(), buttonField), end; it != end; ++it) {
            buttons.push_back(std::stoi((*it)[1].str()));
        }
        s_altSpamDetected = detectAlternation(buttons);
        if (buttons.empty()) {
            log::warn("Alt Spam Helper: unsupported macro format or no recognised button events.");
        } else {
            log::info("Alt Spam Helper: JSON fallback checked {} button events. {}",
                buttons.size(),
                s_altSpamDetected ? "Possible alternating-input pattern found."
                                  : "No sustained alternating-input pattern found.");
        }
    }
}

$on_mod(Loaded) {
    auto selected = Mod::get()->getSettingValue<std::filesystem::path>("macro_file");
    analyseMacro(selected);
    listenForSettingChanges<std::filesystem::path>("macro_file", [](std::filesystem::path const& path) {
        analyseMacro(path);
    });
    log::info("Alt Spam Helper 0.1.2 loaded (legacy GDR1 importer enabled).");
}
