#include <Geode/Geode.hpp>
#include <Geode/loader/SettingV3.hpp>
#include <gdr/gdr.hpp>
#include <algorithm>
#include <fstream>
#include <string>
#include <vector>

using namespace geode::prelude;

namespace {
    struct HelperInput : gdr::Input {
        HelperInput() : gdr::Input(0, 1, false, false) {}
    };

    struct HelperReplay : gdr::Replay<HelperReplay, HelperInput> {
        HelperReplay() : Replay("Alt Spam Helper", "0.1.3") {}
    };

    bool s_altSpamDetected = false;

    bool detectAlternation(std::vector<int> const& buttons) {
        size_t longest = 0, current = 0;
        for (size_t i = 1; i < buttons.size(); ++i) {
            if (buttons[i] != buttons[i - 1]) {
                ++current;
                longest = std::max(longest, current);
            } else current = 0;
        }
        return longest >= 5;
    }

    void report(std::string const& message) {
        log::info("Alt Spam Helper: {}", message);
        Mod::get()->setSettingValue<std::string>("macro_status", message);
    }

    bool analyseJson(std::string const& data, std::vector<int>& buttons) {
        try {
            auto root = gdr::json::parse(data);
            if (!root.is_object() || !root.contains("inputs") || !root["inputs"].is_array())
                return false;
            for (auto const& item : root["inputs"]) {
                if (!item.is_object()) continue;
                // GDR1 uses btn, 2p and down (not button/player2).
                if (item.value("2p", false) || !item.value("down", false)) continue;
                int button = item.value("btn", item.value("button", 1));
                if (button >= 1 && button <= 3) buttons.push_back(button);
            }
            return true;
        } catch (...) {
            return false;
        }
    }

    void analyseMacro(std::filesystem::path const& path) {
        s_altSpamDetected = false;
        if (path.empty()) {
            report("No macro selected.");
            return;
        }

        std::ifstream input(path, std::ios::binary);
        if (!input) {
            report("Could not open selected file. Check file permissions and select it again.");
            return;
        }
        input.seekg(0, std::ios::end);
        auto length = input.tellg();
        if (length <= 0 || length > static_cast<std::streamoff>(16 * 1024 * 1024)) {
            report("File is empty or larger than 16 MB.");
            return;
        }
        input.seekg(0, std::ios::beg);
        std::vector<uint8_t> bytes(static_cast<size_t>(length));
        input.read(reinterpret_cast<char*>(bytes.data()), static_cast<std::streamsize>(bytes.size()));
        if (!input) {
            report("Could not read the complete file.");
            return;
        }

        // GDR2 is a different binary format/API; don't pass it to the GDR1 decoder.
        if (bytes.size() >= 3 && bytes[0] == 'G' && bytes[1] == 'D' && bytes[2] == 'D') {
            report("This appears to be a GDR2 file. GDR2 decoding is not included in this build.");
            return;
        }

        std::vector<int> pressed;
        std::string data(bytes.begin(), bytes.end());
        if (!data.empty() && data.front() == '{' && analyseJson(data, pressed)) {
            s_altSpamDetected = detectAlternation(pressed);
            report("Imported GDR1 JSON: " + std::to_string(pressed.size()) +
                " player-one press events. " +
                (s_altSpamDetected ? "Alternating pattern detected." : "No sustained alternating pattern found."));
            return;
        }

        try {
            auto replay = HelperReplay::importData(bytes, true);
            pressed.reserve(replay.inputs.size());
            for (auto const& event : replay.inputs) {
                if (event.down && !event.player2 && event.button >= 1 && event.button <= 3)
                    pressed.push_back(event.button);
            }
            s_altSpamDetected = detectAlternation(pressed);
            report("Imported GDR1 binary: " + std::to_string(pressed.size()) +
                " player-one press events. " +
                (s_altSpamDetected ? "Alternating pattern detected." : "No sustained alternating pattern found."));
        } catch (std::exception const& error) {
            report(std::string("Import failed: ") + error.what());
        } catch (...) {
            report("Import failed: unsupported or damaged replay file.");
        }
    }
}

$on_mod(Loaded) {
    auto selected = Mod::get()->getSettingValue<std::filesystem::path>("macro_file");
    analyseMacro(selected);
    listenForSettingChanges<std::filesystem::path>("macro_file", [](std::filesystem::path const& path) {
        analyseMacro(path);
    });
    log::info("Alt Spam Helper 0.1.3 loaded.");
}
