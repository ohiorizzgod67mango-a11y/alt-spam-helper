#include "runtime/SpamSession.hpp"
#include <Geode/modify/PlayLayer.hpp>

using namespace geode::prelude;

struct SpamPlayHook : Modify<SpamPlayHook, PlayLayer> {
    bool init(GJGameLevel* level, bool replay, bool skipObjects) {
        if (!PlayLayer::init(level, replay, skipObjects)) return false;
        ash::SpamSession::get().attach(this);
        return true;
    }

    void resetLevel() {
        PlayLayer::resetLevel();
        ash::SpamSession::get().reset(this);
    }

    void postUpdate(float dt) {
        PlayLayer::postUpdate(dt);
        ash::SpamSession::get().drawHud(this);
    }

    void onQuit() {
        ash::SpamSession::get().detach();
        PlayLayer::onQuit();
    }
};
