#include "engine/SpamSmoother.hpp"
#include <algorithm>
#include <cassert>
#include <cmath>
#include <iostream>
#include <random>

using ash::Edge;
using ash::SpamSmoother;

struct Run {
    std::vector<Edge> edges;
    ash::Metrics metrics;
};

Run simulate(std::vector<Edge> const& physical, double strength = 0.85) {
    SpamSmoother engine;
    Run run;
    size_t next = 0;
    double end = physical.back().seconds + 0.1;
    for (int tick = 0; tick / 1000.0 <= end + 1e-9; ++tick) {
        double now = tick / 1000.0;
        auto emitted = engine.pump(now, true);
        run.edges.insert(run.edges.end(), emitted.begin(), emitted.end());
        while (next < physical.size() && physical[next].seconds <= now + 1e-9) {
            auto edge = physical[next++];
            edge.seconds = now;
            emitted = engine.route(edge, {strength, 0.025}, true);
            run.edges.insert(run.edges.end(), emitted.begin(), emitted.end());
        }
    }
    run.metrics = engine.metrics();
    assert(engine.pendingCount() == 0);
    return run;
}

std::vector<Edge> trace(std::vector<double> const& duties) {
    std::vector<Edge> edges;
    for (size_t i = 0; i < duties.size(); ++i) {
        double time = 0.1 + i * 0.08;
        edges.push_back({time,true});
        edges.push_back({time + 0.08 * duties[i],false});
    }
    return edges;
}

double dutyError(std::vector<Edge> const& edges, double target, size_t skip = 8) {
    double sum = 0.0;
    size_t count = 0;
    for (size_t i = skip * 2; i + 2 < edges.size(); i += 2) {
        double period = edges[i + 2].seconds - edges[i].seconds;
        double duty = (edges[i + 1].seconds - edges[i].seconds) / period;
        sum += (duty - target) * (duty - target);
        ++count;
    }
    return std::sqrt(sum / count);
}

int main() {
    SpamSmoother engine;
    assert(engine.pump(10,true).empty());
    std::mt19937 random(42);
    for (int i=0;i<100000;++i) {
        Edge edge{i*0.001,random()%2==0};
        auto output=engine.route(edge,{},false);
        assert(output.size()==1 && output[0].seconds==edge.seconds && output[0].down==edge.down);
        assert(engine.pump(edge.seconds,false).empty());
    }
    std::vector<double> duties(60,0.5);
    for(size_t i=0;i<duties.size();++i) duties[i] += i%2 ? 0.13 : -0.13;
    auto physical=trace(duties);
    auto corrected=simulate(physical);
    assert(corrected.edges.size()==physical.size());
    assert(corrected.metrics.correctedEdges>0);
    double raw=dutyError(physical,0.5);
    double smoothed=dutyError(corrected.edges,0.5);
    assert(smoothed < raw*0.8);
    std::cout<<"alternating jitter rms duty error: "<<raw<<" -> "<<smoothed<<"\n";
    for (double bias : {0.25,0.75}) {
        duties.assign(24,0.5);
        duties.insert(duties.end(),30,bias);
        auto run=simulate(trace(duties));
        assert(std::abs(run.metrics.predictedDuty-bias)<0.03);
        assert(dutyError(run.edges,bias,40)<0.05);
    }
    for (int trial=0;trial<1000;++trial) {
        duties.assign(30,0.5);
        for(auto& duty:duties) duty=0.25+(random()%501)/1000.0;
        auto input=trace(duties);
        auto run=simulate(input);
        assert(run.edges.size()==input.size());
        for(size_t i=0;i<run.edges.size();++i) {
            assert(run.edges[i].down==input[i].down);
            assert(run.edges[i].seconds+1e-9>=input[i].seconds);
            assert(run.edges[i].seconds-input[i].seconds<0.065);
            if(i) assert(run.edges[i].seconds>run.edges[i-1].seconds);
        }
    }
    engine.reset();
    for(auto edge:trace(std::vector<double>(8,0.5))) {
        engine.pump(edge.seconds,true);
        engine.route(edge,{},true);
    }
    auto off=engine.bypass(0.71);
    assert(engine.pendingCount()==0);
    assert(engine.pump(5,false).empty());
    assert(!engine.locked());
    engine.reset();
    assert(engine.route({NAN,true},{},true).empty());
    assert(engine.pump(NAN,true).empty());
    auto zero=engine.route({0,true},{0,0.025},true);
    assert(zero.size()==1&&zero[0].down);
    engine.reset();
    engine.route({0,true},{},true);
    assert(engine.pump(2,true).empty());
    assert(!engine.locked());
    auto release=engine.route({2.01,false},{},true);
    assert(release.size()==1&&!release[0].down);
    std::cout<<"100000 bypass events, 1000 randomized bursts, steering and safety checks passed\n";
}
