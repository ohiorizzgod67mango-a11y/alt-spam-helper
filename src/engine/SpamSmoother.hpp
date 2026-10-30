#pragma once

#include <cstddef>
#include <deque>
#include <string>
#include <vector>

namespace ash {

struct Edge {
    double seconds = 0.0;
    bool down = false;
};

struct Config {
    double strength = 0.85;
    double bufferSeconds = 0.025;
};

struct Metrics {
    size_t physicalEdges = 0;
    size_t correctedEdges = 0;
    size_t cycles = 0;
    double period = 0.0;
    double duty = 0.5;
    double predictedDuty = 0.5;
    double deliveryDelay = 0.0;
    std::string decision = "waiting for spam";
};

class SpamSmoother {
public:
    void reset();
    std::vector<Edge> route(Edge edge, Config config, bool active);
    std::vector<Edge> pump(double now, bool active);
    std::vector<Edge> bypass(double now);
    Metrics const& metrics() const;
    size_t pendingCount() const;
    bool locked() const;

private:
    void emit(std::vector<Edge>& output, Edge edge);
    void cancel(std::vector<Edge>& output, double now);
    void clearModel();
    void observe(Edge edge);
    std::deque<Edge> m_pending;
    Metrics m_metrics;
    bool m_physical = false;
    bool m_output = false;
    bool m_active = false;
    bool m_havePress = false;
    bool m_haveRelease = false;
    bool m_havePhase = false;
    double m_press = 0.0;
    double m_release = 0.0;
    double m_phase = 0.0;
    double m_lastEdge = -1.0;
    double m_lastDelivered = -1.0;
    double m_lastDuty = 0.5;
    double m_lastDutyDelta = 0.0;
};

}
