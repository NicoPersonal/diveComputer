#ifndef SET_POINTS_HPP
#define SET_POINTS_HPP

#include "global.hpp"
#include "error_handler.hpp"

namespace DiveComputer {

class SetPoints {
public:
    SetPoints();

    // Attributes
    std::vector<double> m_depths;
    std::vector<double> m_setPoints;

    // Methods  
    double getSetPointAtDepth(double depth, bool boosted);

    // File operations
    void setToDefault();
    bool loadSetPointsFromFile();
    bool saveSetPointsToFile();
    void sortSetPoints();
    size_t nbOfSetPoints() const { return m_depths.size(); }
    void addSetPoint(double depth, double setpoint);
    void removeSetPoint(size_t index);
};

} // namespace DiveComputer

#endif
