#ifndef STOP_STEPS_HPP
#define STOP_STEPS_HPP

#include <vector>
#include <iostream>

namespace DiveComputer {

// Stop step
class StopStep {
public:
    StopStep(double depth, double time);
    ~StopStep() = default;

    double m_depth;
    double m_time;
};

// Stop steps class
class StopSteps {
public:
    StopSteps();
    ~StopSteps() = default;

    std::vector<StopStep> m_stopSteps;
    int  nbOfStopSteps();
    void addStopStep(double depth, double time);
    void removeStopStep(int index);
    void clear();
    void editStopStep(int index, double depth, double time);
    double maxDepth();
    void sortDescending();
    void sortAscending();
    void print();
};

} // namespace DiveComputer

#endif // STOP_STEPS_HPP