#include <Geode/Geode.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include <Geode/loader/SettingV3.hpp>
#include <set>
#include <algorithm>
#include <fstream>
#include <regex>
#include <string>

using namespace geode::prelude;

namespace {
    bool s_twoFingerHeld = false;
    bool s_altSpamDetected = false;

    bool helperEnabled() {
        return Mod::get()->getSettingValue<bool>("helper_enabled") && s_altSpamDetected;
    }

    int tapRate() {
        return std::clamp(
            static_cast<int>(Mod::get()->getSettingValue<int64_t>("spam_cps")),
            2, 30
        );
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

        std::string data(
            (std::istreambuf_iterator<char>(input)),
            std::istreambuf_iterator<char>()
        );

        // Initial JSON-style event parser. It looks for a sequence of alternating
        // button 1 and button 2 events. Other formats need dedicated parsers.
        static const std::regex buttonField(R"("button"\s*:\s*([12]))");
        std::vector<int> buttons;
        for (std::sregex_iterator it(data.begin(), data.end(), buttonField), end; it != end; ++it) {
            buttons.push_back(std::stoi((*it)[1].str()));
        }

        int alternating = 0;
        for (size_t i = 1; i < buttons.size(); ++i) {
            if (buttons[i] != buttons[i - 1]) {
                ++alternating;
            } else {
                alternating = 0;
            }
            if (alternating >= 5) {
                s_altSpamDetected = true;
                break;
            }
        }

        if (s_altSpamDetected) {
            log::info("Alt Spam Helper: alternating button pattern detected. Alt Spam helper enabled.");
        } else if (buttons.empty()) {
            log::warn("Alt Spam Helper: file selected, but no supported JSON button events were found.");
        } else {
            log::info("Alt Spam Helper: no sustained alternating button pattern found.");
        }
    }
}

class TwoFingerDetector final : public CCLayer {
    std::set<int> m_touchIDs;

protected:
    bool init() {
        if (!CCLayer::init()) return false;
        setTouchEnabled(true);
        setTouchMode(kCCTouchesAllAtOnce);
        setTouchPriority(-500);
        setContentSize(CCDirector::sharedDirector()->getWinSize());
        return true;
    }

public:
    static TwoFingerDetector* create() {
        auto ret = new TwoFingerDetector();
        if (ret && ret->init()) {
            ret->autorelease();
            return ret;
        }
        CC_SAFE_DELETE(ret);
        return nullptr;
    }

    void ccTouchesBegan(CCSet* touches, CCEvent*) override {
        for (auto it = touches->begin(); it != touches->end(); ++it) {
            auto touch = static_cast<CCTouch*>(*it);
            m_touchIDs.insert(touch->getID());
        }
        s_twoFingerHeld = helperEnabled() && m_touchIDs.size() >= 2;
    }

    void ccTouchesEnded(CCSet* touches, CCEvent*) override {
        for (auto it = touches->begin(); it != touches->end(); ++it) {
            auto touch = static_cast<CCTouch*>(*it);
            m_touchIDs.erase(touch->getID());
        }
        s_twoFingerHeld = helperEnabled() && m_touchIDs.size() >= 2;
    }

    void ccTouchesCancelled(CCSet*, CCEvent*) override {
        m_touchIDs.clear();
        s_twoFingerHeld = false;
    }

    void onExit() override {
        m_touchIDs.clear();
        s_twoFingerHeld = false;
        CCLayer::onExit();
    }
};

class $modify(AltSpamHelperPlayLayer, PlayLayer) {
    bool init(GJGameLevel* level, bool useReplay, bool dontCreateObjects) {
        if (!PlayLayer::init(level, useReplay, dontCreateObjects)) return false;
        s_twoFingerHeld = false;
        if (auto detector = TwoFingerDetector::create()) {
            addChild(detector, -1000);
        }
        return true;
    }

    void onExit() {
        s_twoFingerHeld = false;
        PlayLayer::onExit();
    }
};

$on_mod(Loaded) {
    auto selected = Mod::get()->getSettingValue<std::filesystem::path>("macro_file");
    analyseMacro(selected);

    listenForSettingChanges("macro_file", [](std::filesystem::path const& path) {
        analyseMacro(path);
    });

    log::info("Alt Spam Helper prototype loaded. Tap rate setting: {}", tapRate());
}
