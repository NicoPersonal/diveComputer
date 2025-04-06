#ifndef dive_step_HPP
#define dive_step_HPP

#include <string>
#include <memory>
#include <vector>
#include <cstddef>
#include "enum.hpp"
#include <iostream>
#include <iomanip>
#include "compartments.hpp"
#include "buhlmann.hpp"
#include "global.hpp"
#include "oxygen_toxicity.hpp"
#include "gas.hpp"

namespace DiveComputer {

// Dive profile step
class DiveStep {
public:
    DiveStep() = default;
    ~DiveStep() = default;

    friend std::ostream& operator<<(std::ostream& os, const DiveStep& step);
    
    Phase  m_phase{Phase::STOP}; 
    stepMode m_mode{stepMode::OC};

    double m_startDepth{0.0};
    double m_endDepth{0.0};

    double m_time{0.0};
    double m_runTime{0.0};
    
    double m_pAmbStartDepth{0.0};
    double m_pAmbEndDepth{0.0};
    double m_pAmbMax{0.0};
    double m_pO2Max{0.0};

    double m_o2Percent{0.0};
    double m_n2Percent{0.0};
    double m_hePercent{0.0};

    double m_gf{0.0};
    double m_gfSurface{0.0};

    std::vector<CompartmentPP> m_ppMax{NUM_COMPARTMENTS};
    std::vector<CompartmentPP> m_ppMaxAdjustedGF{NUM_COMPARTMENTS};
    std::vector<CompartmentPP> m_ppActual{NUM_COMPARTMENTS};

    double m_sacRate{0.0};
    double m_ambConsumptionAtDepth{0.0};
    double m_stepConsumption{0.0};

    double m_gasDensity{0.0};
    double m_endWithoutO2{0.0};
    double m_endWithO2{0.0};

    double m_cnsMaxMinSingleDive{0.0};
    double m_cnsStepSingleDive{0.0};
    double m_cnsTotalSingleDive{0.0};

    double m_cnsMaxMinMultipleDives{0.0};
    double m_cnsStepMultipleDives{0.0};
    double m_cnsTotalMultipleDives{0.0};

    double m_otuPerMin{0.0};
    double m_otuStep{0.0};
    double m_otuTotal{0.0};

    double m_ceiling{0.0};

    // Core functions    
    double getGFSurface(DiveStep *stepSurface);
    double getCeiling(double GF);
    void   calculatePPInertGasForStep(DiveStep& previousStep, double time);
    void   calculatePPInertGasMaxForStep(double& lastRatioN2He);
    bool   getIfBreachingDecoLimits();

    // update functions
    void updatePAmb();
    void updateCeiling(double GF);
    void updateOxygenToxicity(DiveStep *previousStep);
    void updateConsumption();
    void updateGFSurface(DiveStep *stepSurface);
    void updateDensity();
    void updateEND();
    void updateRunTime(DiveStep *previousDiveStep);

    // Print to terminal functions
    void printStepDetails(const int step) const;
    void printCompartmentDetails(const int step, const int compartment) const;

};

} // namespace DiveComputer
#endif // dive_step_HPP
