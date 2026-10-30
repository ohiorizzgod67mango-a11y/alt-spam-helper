#include "PauseEntry.hpp"

#include "ui/SpamPopup.hpp"
#include <Geode/binding/ButtonSprite.hpp>

using namespace geode::prelude;

namespace ash {

void PauseEntry::attach(PauseLayer* layer) {
    if (layer->getChildByID("alt-spam-helper-entry"_spr)) return;
    auto* controller = new PauseEntry();
    if (!controller->init()) {
        delete controller;
        return;
    }
    controller->autorelease();
    controller->setID("alt-spam-helper-entry"_spr);
    layer->addChild(controller);
    auto* host = typeinfo_cast<CCMenu*>(layer->getChildByID("right-button-menu"));
    if (!host) {
        host = CCMenu::create();
        auto size = CCDirector::sharedDirector()->getWinSize();
        host->setPosition({size.width - 50.f, 140.f});
        layer->addChild(host);
    }
    auto* sprite = ButtonSprite::create("spam");
    sprite->setScale(0.55f);
    auto* button = CCMenuItemSpriteExtra::create(sprite, controller, menu_selector(PauseEntry::onOpen));
    button->setID("alt-spam-helper-button"_spr);
    host->addChild(button);
    host->updateLayout();
}

void PauseEntry::onOpen(CCObject*) {
    if (auto* popup = SpamPopup::create()) popup->show();
}

}
