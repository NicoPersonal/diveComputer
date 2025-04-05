#ifndef PARAMETERS_HPP
#define PARAMETERS_HPP

#include "global.hpp"
#include <iostream>
#include <fstream>
#include <filesystem>
#include <string>

// If you have a different path definition in a constants file, you'd include it here
// For now I'll assume PATH_PARAMETERS is defined elsewhere or we'll replace it

namespace DiveComputer {

class Parameters {
public:
    Parameters();
    
    void setToDefault();
    bool loadParametersFromFile();
    bool saveParametersToFile();
    
    // Your parameter variables
    double m_gf[2]; 
    double m_atmPressure;
    double m_tempMin;
    double m_defaultEnd;
    bool   m_defaultO2Narcotic;
    double m_maxAscentRate;
    double m_maxDescentRate;
    double m_sacBottom;
    double m_sacBailout;
    double m_sacDeco;
    double m_o2CostPerL;
    double m_heCostPerL;
    double m_bestMixDepthBuffer;
    double m_PpO2Active;
    double m_PpO2Deco;
    double m_maxPpO2Diluent;
    double m_warningPpO2Low;
    double m_warningCnsMax;
    double m_warningOtuMax;
    double m_warningGasDensity;
    double m_depthIncrement;
    double m_lastStopDepth;
    double m_timeIncrementDeco;
    double m_timeIncrementMaxTime;
    double m_noFlyPressure;
    double m_noFlyGf;
    double m_noFlyTimeIncrement;
};

// Global parameters object
extern Parameters g_parameters;

} // namespace DiveComputer

#endif // PARAMETERS_HPP