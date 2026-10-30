#include "SpamPopup.hpp"
#include "runtime/SpamSession.hpp"
#include <Geode/binding/ButtonSprite.hpp>
#include <algorithm>

using namespace geode::prelude;

namespace ash {

SpamPopup* SpamPopup::create() {
    auto* popup = new SpamPopup();
    if (popup->init()) {
        popup->autorelease();
        return popup;
    }
    delete popup;
    return nullptr;
}

bool SpamPopup::init() {
    if (!Popup::init(420.f, 300.f, "GJ_square01.png")) return false;
    setTitle("wave spam helper", "goldFont.fnt", 0.8f);
    auto label = [this](float y, float scale) {
        auto* node = CCLabelBMFont::create("", "bigFont.fnt");
        node->setPosition({210.f, y});
        node->setScale(scale);
        m_mainLayer->addChild(node);
        return node;
    };
    m_status = label(240.f, 0.28f);
    m_strength = label(160.f, 0.35f);
    m_buffer = label(115.f, 0.35f);
    m_diagnostic = label(69.f, 0.19f);
    auto* note = label(32.f, 0.23f);
    note->setString("wave only | normal mode supported | assisted run");
    m_controls = CCMenu::create();
    m_controls->setPosition({0.f, 0.f});
    m_mainLayer->addChild(m_controls);
    addButton("enable", 150.f, 203.f, menu_selector(SpamPopup::onEnable));
    addButton("off", 285.f, 203.f, menu_selector(SpamPopup::onOff));
    addButton("-", 62.f, 160.f, menu_selector(SpamPopup::onStrength), -1);
    addButton("+", 358.f, 160.f, menu_selector(SpamPopup::onStrength), 1);
    addButton("-5ms", 65.f, 115.f, menu_selector(SpamPopup::onBuffer), -1);
    addButton("+5ms", 355.f, 115.f, menu_selector(SpamPopup::onBuffer), 1);
    refresh();
    return true;
}

void SpamPopup::addButton(char const* text, float x, float y, SEL_MenuHandler callback, int tag) {
    auto* sprite = ButtonSprite::create(text);
    sprite->setScale(0.55f);
    auto* button = CCMenuItemSpriteExtra::create(sprite, this, callback);
    button->setPosition({x, y});
    button->setTag(tag);
    m_controls->addChild(button);
}

void SpamPopup::refresh() {
    auto* mod = Mod::get();
    m_status->setString(SpamSession::get().status().c_str());
    m_status->limitLabelWidth(380.f, 0.28f, 0.17f);
    m_strength->setString(fmt::format("strength {:.0f}%", mod->getSettingValue<double>("strength") * 100.0).c_str());
    m_buffer->setString(fmt::format("buffer {} ms", mod->getSettingValue<int64_t>("buffer-ms")).c_str());
    m_diagnostic->setString(SpamSession::get().summary().c_str());
    m_diagnostic->limitLabelWidth(380.f, 0.19f, 0.12f);
}

void SpamPopup::onEnable(CCObject*) {
    auto error = SpamSession::get().enable();
    if (!error.empty()) FLAlertLayer::create("not enabled", error, "ok")->show();
    refresh();
}

void SpamPopup::onOff(CCObject*) {
    SpamSession::get().disable();
    refresh();
}

void SpamPopup::onStrength(CCObject* sender) {
    SpamSession::get().clearPending();
    auto* mod = Mod::get();
    double value = mod->getSettingValue<double>("strength") + static_cast<CCNode*>(sender)->getTag() * 0.05;
    mod->setSettingValue<double>("strength", std::clamp(value, 0.0, 1.0));
    refresh();
}

void SpamPopup::onBuffer(CCObject* sender) {
    SpamSession::get().clearPending();
    auto* mod = Mod::get();
    auto value = mod->getSettingValue<int64_t>("buffer-ms") + static_cast<CCNode*>(sender)->getTag() * 5;
    mod->setSettingValue<int64_t>("buffer-ms", std::clamp<int64_t>(value, 5, 60));
    refresh();
}

}
