#include "ui/PauseEntry.hpp"
#include <Geode/modify/PauseLayer.hpp>

using namespace geode::prelude;

struct SpamPauseHook : Modify<SpamPauseHook, PauseLayer> {
    void customSetup() {
        PauseLayer::customSetup();
        ash::PauseEntry::attach(this);
    }
};
