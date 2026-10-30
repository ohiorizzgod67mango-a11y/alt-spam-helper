#pragma once

#include <Geode/Geode.hpp>
#include <Geode/ui/Popup.hpp>

namespace ash {

class SpamPopup : public geode::Popup {
public:
    static SpamPopup* create();

protected:
    bool init();

private:
    void refresh();
    void addButton(char const* text, float x, float y, cocos2d::SEL_MenuHandler callback, int tag = 0);
    void onEnable(cocos2d::CCObject*);
    void onOff(cocos2d::CCObject*);
    void onStrength(cocos2d::CCObject* sender);
    void onBuffer(cocos2d::CCObject* sender);
    cocos2d::CCMenu* m_controls = nullptr;
    cocos2d::CCLabelBMFont* m_status = nullptr;
    cocos2d::CCLabelBMFont* m_strength = nullptr;
    cocos2d::CCLabelBMFont* m_buffer = nullptr;
    cocos2d::CCLabelBMFont* m_diagnostic = nullptr;
};

}
