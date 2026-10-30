#include "SpamSmoother.hpp"
#include <algorithm>
#include <cmath>

namespace ash {

void SpamSmoother::clearModel() {
    m_metrics.cycles = 0;
    m_metrics.period = 0.0;
    m_metrics.duty = 0.5;
    m_metrics.predictedDuty = 0.5;
    m_havePress = false;
    m_haveRelease = false;
    m_havePhase = false;
    m_lastDuty = 0.5;
    m_lastDutyDelta = 0.0;
}

void SpamSmoother::reset() {
    m_pending.clear();
    m_metrics = {};
    m_physical = false;
    m_output = false;
    m_active = false;
    m_lastEdge = -1.0;
    m_lastDelivered = -1.0;
    clearModel();
}

void SpamSmoother::emit(std::vector<Edge>& output, Edge edge) {
    m_output = edge.down;
    m_lastDelivered = edge.seconds;
    output.push_back(edge);
}

void SpamSmoother::cancel(std::vector<Edge>& output, double now) {
    m_pending.clear();
    if (m_output != m_physical) emit(output, {now, m_physical});
    clearModel();
    m_active = false;
}

std::vector<Edge> SpamSmoother::bypass(double now) {
    std::vector<Edge> output;
    if (std::isfinite(now)) cancel(output, now);
    return output;
}

void SpamSmoother::observe(Edge edge) {
    if (edge.down) {
        if (m_havePress && m_haveRelease) {
            double period = edge.seconds - m_press;
            double hold = m_release - m_press;
            if (period >= 0.025 && period <= 1.0 / 6.0 && hold > 0.001 && hold < period - 0.001) {
                double duty = std::clamp(hold / period, 0.08, 0.92);
                double delta = duty - m_lastDuty;
                bool trend = delta * m_lastDutyDelta > 0.0001;
                double response = trend ? 0.7 : 0.3;
                if (m_metrics.cycles == 0) {
                    m_metrics.period = period;
                    m_metrics.duty = duty;
                } else {
                    m_metrics.period += (period - m_metrics.period) * 0.4;
                    m_metrics.duty += (duty - m_metrics.duty) * response;
                }
                double prediction = trend ? std::clamp(delta, -0.08, 0.08) * 0.5 : 0.0;
                m_metrics.predictedDuty = std::clamp(m_metrics.duty + prediction, 0.08, 0.92);
                m_lastDutyDelta = delta;
                m_lastDuty = duty;
                ++m_metrics.cycles;
            } else clearModel();
        }
        m_press = edge.seconds;
        m_havePress = true;
        m_haveRelease = false;
    } else if (m_havePress) {
        m_release = edge.seconds;
        m_haveRelease = true;
    }
}

std::vector<Edge> SpamSmoother::route(Edge edge, Config config, bool active) {
    std::vector<Edge> output;
    if (!std::isfinite(edge.seconds)) return output;
    bool usable = active && std::isfinite(config.strength) && std::isfinite(config.bufferSeconds) &&
        config.strength > 0.0 && config.strength <= 1.0 && config.bufferSeconds >= 0.005 && config.bufferSeconds <= 0.06;
    bool previous = m_physical;
    m_physical = edge.down;
    ++m_metrics.physicalEdges;
    if (!usable) {
        if (m_active) cancel(output, edge.seconds);
        emit(output, edge);
        m_metrics.decision = "bypass";
        return output;
    }
    m_active = true;
    if (previous == edge.down) {
        if (m_pending.empty()) emit(output, edge);
        m_metrics.decision = "repeat";
        return output;
    }
    if (m_lastEdge >= 0.0 && edge.seconds < m_lastEdge) {
        cancel(output, edge.seconds);
        m_lastEdge = edge.seconds;
        m_metrics.decision = "clock moved backwards";
        return output;
    }
    m_lastEdge = edge.seconds;
    bool wasLocked = locked();
    observe(edge);
    if (!locked()) {
        if (wasLocked || !m_pending.empty()) {
            cancel(output, edge.seconds);
            if (output.empty() || output.back().down != edge.down) emit(output, edge);
        } else emit(output, edge);
        m_metrics.decision = "learning spam rhythm";
        return output;
    }
    if (m_pending.size() >= 64) {
        cancel(output, edge.seconds);
        m_metrics.decision = "overload, physical control restored";
        return output;
    }
    double base = edge.seconds + config.bufferSeconds;
    double target = base;
    if (m_havePhase) {
        double predicted = m_phase + (edge.down ? m_metrics.period : m_metrics.period * m_metrics.predictedDuty);
        double correction = std::clamp((predicted - base) * config.strength,
            -config.bufferSeconds * 0.8, config.bufferSeconds * 0.8);
        target = base + correction;
    }
    target = std::max(target, edge.seconds + 0.001);
    if (!m_pending.empty()) target = std::max(target, m_pending.back().seconds + 0.001);
    target = std::max(target, m_lastDelivered + 0.001);
    if (edge.down) {
        m_phase = target;
        m_havePhase = true;
    }
    m_pending.push_back({target, edge.down});
    ++m_metrics.correctedEdges;
    m_metrics.decision = "smoothing spam and following steering";
    return output;
}

std::vector<Edge> SpamSmoother::pump(double now, bool active) {
    std::vector<Edge> output;
    if (!std::isfinite(now)) return output;
    if (!active) {
        if (m_active) cancel(output, now);
        return output;
    }
    double timeout = std::max(0.14, m_metrics.period * 1.8);
    if (m_active && m_lastEdge >= 0.0 && now - m_lastEdge > timeout) {
        cancel(output, now);
        m_metrics.decision = "spam stopped, physical control restored";
        return output;
    }
    if (m_pending.empty() || now <= m_lastDelivered + 1e-9 || m_pending.front().seconds > now + 1e-9) return output;
    auto edge = m_pending.front();
    m_pending.pop_front();
    m_metrics.deliveryDelay = std::max(0.0, now - edge.seconds);
    edge.seconds = now;
    emit(output, edge);
    return output;
}

Metrics const& SpamSmoother::metrics() const { return m_metrics; }
size_t SpamSmoother::pendingCount() const { return m_pending.size(); }
bool SpamSmoother::locked() const { return m_metrics.cycles >= 2; }

}
