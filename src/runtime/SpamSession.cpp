#include "SpamSession.hpp"
#include <algorithm>
#include <cmath>

using namespace geode::prelude;

namespace ash {

SpamSession& SpamSession::get() {
    static SpamSession session;
    return session;
}

void SpamSession::attach(PlayLayer* layer) {
    detach();
    m_layer = layer;
    m_label = CCLabelBMFont::create("spam helper | off", "bigFont.fnt");
    m_label->setPosition({8.f, CCDirector::sharedDirector()->getWinSize().height - 8.f});
    m_label->setAnchorPoint({0.f, 1.f});
    m_label->setScale(0.26f);
    layer->addChild(m_label, 1000);
}

void SpamSession::detach() {
    m_layer = nullptr;
    m_label = nullptr;
    m_dispatching = false;
    for (auto& engine : m_engines) engine.reset();
}

void SpamSession::reset(PlayLayer* layer) {
    if (layer != m_layer) return;
    for (auto& engine : m_engines) engine.reset();
}

Config SpamSession::config() const {
    auto* mod = Mod::get();
    return {std::clamp(mod->getSettingValue<double>("strength"), 0.0, 1.0),
            std::clamp(mod->getSettingValue<int64_t>("buffer-ms") / 1000.0, 0.005, 0.06)};
}

bool SpamSession::active(bool player1) const {
    if (!m_layer || !Mod::get()->getSettingValue<bool>("enabled")) return false;
    if (config().strength <= 0.0) return false;
    if (!player1 && !m_layer->m_gameState.m_isDualMode) return false;
    auto* player = player1 ? m_layer->m_player1 : m_layer->m_player2;
    return player && player->m_isDart;
}

void SpamSession::dispatch(bool player1, std::vector<Edge> const& edges) {
    if (!m_layer || edges.empty()) return;
    m_dispatching = true;
    for (auto const& edge : edges) m_layer->handleButton(edge.down, 1, player1);
    m_dispatching = false;
}

bool SpamSession::input(PlayLayer* layer, bool down, int button, bool player1) {
    if (m_dispatching || layer != m_layer || button != 1) return false;
    double now = layer->m_gameState.m_levelTime;
    if (!std::isfinite(now)) return false;
    auto& engine = m_engines[player1 ? 0 : 1];
    auto edges = engine.route({now, down}, config(), active(player1));
    if (!edges.empty() && edges.back().seconds == now && edges.back().down == down) {
        edges.pop_back();
        std::erase_if(edges, [down](Edge const& edge) { return edge.down == down; });
        dispatch(player1, edges);
        return false;
    }
    dispatch(player1, edges);
    return true;
}

void SpamSession::tick(PlayLayer* layer) {
    if (layer != m_layer) return;
    double now = layer->m_gameState.m_levelTime;
    for (size_t i = 0; i < m_engines.size(); ++i) dispatch(i == 0, m_engines[i].pump(now, active(i == 0)));
}

void SpamSession::clearPending() {
    if (!m_layer) return;
    for (size_t i = 0; i < m_engines.size(); ++i) dispatch(i == 0, m_engines[i].bypass(m_layer->m_gameState.m_levelTime));
}

std::string SpamSession::enable() {
    if (!m_layer) return "enter a level first";
    clearPending();
    Mod::get()->setSettingValue<bool>("enabled", true);
    return {};
}

void SpamSession::disable() {
    clearPending();
    Mod::get()->setSettingValue<bool>("enabled", false);
}

std::string SpamSession::status() const {
    if (!m_layer) return "enter a level";
    if (!Mod::get()->getSettingValue<bool>("enabled")) return "off";
    if (config().strength <= 0.0) return "zero strength, bypass";
    if (!active(true) && !active(false)) return "waiting for wave mode";
    return "assisted wave, normal mode supported";
}

std::string SpamSession::summary() const {
    auto const& engine = m_engines[0];
    auto const& metrics = engine.metrics();
    double cps = metrics.period > 0.0 ? 1.0 / metrics.period : 0.0;
    return fmt::format("{} | {:.1f} cps | hold {:.0f}% | adjusted {} | queue {}",
        metrics.decision, cps, metrics.predictedDuty * 100.0, metrics.correctedEdges, engine.pendingCount());
}

void SpamSession::drawHud(PlayLayer* layer) {
    if (layer != m_layer || !m_label) return;
    std::string text = "spam helper | " + status() + " | " + summary();
    bool details = Mod::get()->getSettingValue<bool>("show-hud");
    if (!details) text = "spam helper | assisted wave";
    m_label->setString(text.c_str());
    m_label->limitLabelWidth(CCDirector::sharedDirector()->getWinSize().width - 16.f, 0.26f, 0.12f);
    m_label->setVisible(details || active(true) || active(false));
    m_label->setColor(active(true) || active(false) ? ccColor3B{190, 245, 120} : ccColor3B{230, 190, 130});
}

}
