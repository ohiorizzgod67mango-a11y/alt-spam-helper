#include "runtime/SpamSession.hpp"
#include <Geode/modify/GJBaseGameLayer.hpp>

using namespace geode::prelude;

struct SpamInputHook : Modify<SpamInputHook, GJBaseGameLayer> {
    void handleButton(bool down, int button, bool player1) {
        auto* play = PlayLayer::get();
        if (play && static_cast<GJBaseGameLayer*>(play) == static_cast<GJBaseGameLayer*>(this) &&
            ash::SpamSession::get().input(play, down, button, player1)) return;
        GJBaseGameLayer::handleButton(down, button, player1);
    }

    void processCommands(float dt, bool halfTick, bool lastTick) {
        auto* play = PlayLayer::get();
        if (play && static_cast<GJBaseGameLayer*>(play) == static_cast<GJBaseGameLayer*>(this)) {
            ash::SpamSession::get().tick(play);
        }
        GJBaseGameLayer::processCommands(dt, halfTick, lastTick);
    }
};
