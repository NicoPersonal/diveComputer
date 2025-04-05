#include "stop_steps.hpp"

namespace DiveComputer {

// Constructor

StopStep::StopStep(double depth, double time) {
    m_depth = depth;
    m_time = time;
}

StopSteps::StopSteps() {}

int StopSteps::nbOfStopSteps() {
    return m_stopSteps.size();
}

void StopSteps::addStopStep(double depth, double time) {
    m_stopSteps.push_back(StopStep(depth, time));
    sortDescending();
}

void StopSteps::removeStopStep(int index) {
    m_stopSteps.erase(m_stopSteps.begin() + index);
}

void StopSteps::clear() {
    m_stopSteps.clear();
}

void StopSteps::editStopStep(int index, double depth, double time) {
    m_stopSteps[index] = StopStep(depth, time);
    sortDescending();
}

double StopSteps::maxDepth() {
    if (m_stopSteps.empty()) {
        return 0.0;
    }
    
    double maxDepth = m_stopSteps[0].m_depth;
    for (const auto& step : m_stopSteps) {
        if (step.m_depth > maxDepth) {
            maxDepth = step.m_depth;
        }
    }
    return maxDepth;
}

void StopSteps::sortDescending() {
    std::sort(m_stopSteps.begin(), m_stopSteps.end(), [](const StopStep& a, const StopStep& b) {
        return a.m_depth > b.m_depth;
    });
}

void StopSteps::sortAscending() {
    std::sort(m_stopSteps.begin(), m_stopSteps.end(), [](const StopStep& a, const StopStep& b) {
        return a.m_depth < b.m_depth;
    });
}

void StopSteps::print() {
    for (const auto& step : m_stopSteps) {
        std::cout << "Depth: " << step.m_depth << " Time: " << step.m_time << std::endl;
    }
}

} // namespace DiveComputer