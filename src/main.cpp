#include <Geode/Geode.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include <set>
#include <algorithm>

using namespace geode::prelude;

namespace {
    bool s_twoFingerHeld = false;

    bool helperEnabled() {
        return Mod::get()->getSettingValue<bool>("helper_enabled");
    }

    int tapRate() {
        return std::clamp(
            static_cast<int>(Mod::get()->getSettingValue<int64_t>("spam_cps")),
            2, 30
        );
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

// The first build establishes the invisible two-finger detector and settings.
// Automated playback is kept disabled until the Android input hooks are verified.
$on_mod(Loaded) {
    log::info("Alt Spam Helper prototype loaded. Tap rate setting: {}", tapRate());
}
