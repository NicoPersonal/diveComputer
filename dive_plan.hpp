#ifndef DIVE_PLAN_HPP
#define DIVE_PLAN_HPP

#include <vector>
#include <memory>
#include <set>

#include "enum.hpp"
#include "dive_step.hpp"
#include "stop_steps.hpp"
#include "compartments.hpp"
#include "parameters.hpp"
#include "gas.hpp"
#include "gaslist.hpp"
#include "set_points.hpp"
#include "oxygen_toxicity.hpp"

namespace DiveComputer {

// Create a new struct for gas tracking
struct GasAvailable {
    Gas    m_gas;
    double m_switchDepth;
    double m_switchPpO2;
    int    m_nbTanks;
    double m_tankCapacity;    // in liters
    double m_fillingPressure; // in bar
    double m_reservePressure; // in bar
    double m_consumption;     // accumulated during dive
    double m_endPressure;     // calculated at end of dive
    
    GasAvailable(const Gas& g) : m_gas(g), m_nbTanks(1), m_tankCapacity(11.0), 
                                m_fillingPressure(200.0), m_reservePressure(70.0),
                                m_consumption(0.0), m_endPressure(200.0) {}
};

// Dive profile management class
class DivePlan {
public:
    DivePlan(double depth, double time, diveMode mode, int diveNumber, std::vector<CompartmentPP> initialPressure);
    ~DivePlan() = default;

    StopSteps m_stopSteps;
    diveMode  m_mode;

    bool m_bailout;
    int  m_diveNumber;
    bool m_boosted;
    SetPoints m_setPoints;

    std::vector<CompartmentPP> m_initialPressure;
    std::vector<DiveStep> m_diveProfile;
    std::vector<DiveStep> m_timeProfile;
    std::vector<GasAvailable> m_gasAvailable;

    // Core methods
    void loadAvailableGases();
    void build();
    void calculate();
    void calculateOtherVariables();
    void updateGasConsumption();
    int  nbOfSteps();

    // Action methods
    void   defineMission();
    void   setMaxTime();
    void   optimiseDecoGas();
    double getTTSMax();
    double getTTS();
    double getTTSDelta();
    double getTP();

    // Print-to-terminal functions
    void printPlan(std::vector<DiveStep> profile);
    void printCompartmentDetails(int compartment);
    void printStepDetails(int step);
    void printGF();
    void printO2Exposure();

private:
    double m_firstDecoDepth;

    // Helper methods
    void   clear();
    void   sortGases();
    void   applyGases();
    void   calculateDecoSteps();
    double calculateFirstStopDepth(double maxDepth);
    void   processAscentStops(const std::vector<double>& ascentStops);

    DiveStep& addStep(double start_depth, double end_depth, double time, Phase phase, stepMode mode);
    DiveStep& insertStep(int index, double start_depth, double end_depth, double time, Phase phase, stepMode mode);
    void deleteStep(int index);

    // Decompression methods
    void calculatePPInertGas();
    void calculatePPInertGasMax();
    void applyGF();
    void setFirstDecoDepth();

    // update variable functions
    void updateStepsPhaseFromFirstDeco();
    void updatePpAmb();
    void updateCeiling(double GF);
    void updateOxygenToxicity();
    void updateConsumptions();
    void updateGFSurface();
    void updateRunTimes();
    void updateVariablesAtOnce();
    void updateVariables(double GF);
    void updateTimeProfile();
};

} // namespace DiveComputer

#endif // DIVE_PLAN_HPP
