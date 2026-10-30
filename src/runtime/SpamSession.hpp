#pragma once

#include "engine/SpamSmoother.hpp"
#include <Geode/Geode.hpp>
#include <array>

namespace ash {

class SpamSession {
public:
    static SpamSession& get();
    void attach(PlayLayer* layer);
    void detach();
    void reset(PlayLayer* layer);
    void tick(PlayLayer* layer);
    void drawHud(PlayLayer* layer);
    bool input(PlayLayer* layer, bool down, int button, bool player1);
    std::string enable();
    void disable();
    void clearPending();
    std::string status() const;
    std::string summary() const;

private:
    bool active(bool player1) const;
    Config config() const;
    void dispatch(bool player1, std::vector<Edge> const& edges);
    std::array<SpamSmoother, 2> m_engines;
    PlayLayer* m_layer = nullptr;
    cocos2d::CCLabelBMFont* m_label = nullptr;
    bool m_dispatching = false;
};

}
