#pragma once

#include <Geode/Geode.hpp>

namespace ash {

class PauseEntry : public cocos2d::CCNode {
public:
    static void attach(PauseLayer* layer);

private:
    void onOpen(cocos2d::CCObject*);
};

}
