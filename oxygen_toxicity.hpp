#ifndef OXYGEN_TOXICITY_HPP
#define OXYGEN_TOXICITY_HPP

#include <array>

namespace DiveComputer {

// Oxygen toxicity calculations class

// Oxygen exposure parameters
class O2Exposure {
public:
    O2Exposure() = default;
    O2Exposure(double ppO2Start, double ppO2End, 
               double aCNSMaxMinSingleDive, double bCNSMaxMinSingleDive,
               double aCNSMaxMinMultipleDives, double bCNSMaxMinMultipleDives);
    
    double m_ppO2Start{0.0};
    double m_ppO2End{0.0};
    
    double m_aCNSMaxMinSingleDive{0.0};
    double m_bCNSMaxMinSingleDive{0.0};
 
    double m_aCNSMaxMinMultipleDives{0.0};
    double m_bCNSMaxMinMultipleDives{0.0};
};


class OxygenToxicity {
public:
    OxygenToxicity();
    ~OxygenToxicity() = default;
    
    // Calculate OTU per minute
    double getOTUPerMin(double ppO2Ambient) const;
    
    // Calculate CNS max minutes (single_dive or multiple_dives)
    double getCNSMaxMin(double ppO2Ambient, bool singleDive) const;

private:
    // NOAA table linearized in the form of CNS_total = a * ppO2 + b
    static constexpr int NUM_O2_EXPOSURE_PARAMETERS = 6;
    std::array<O2Exposure, NUM_O2_EXPOSURE_PARAMETERS> m_o2ExposureParameters;
};

// Global instance - will be defined in oxygen_toxicity.cpp
extern OxygenToxicity g_oxygenToxicity;

} // namespace DiveComputer

#endif // OXYGEN_TOXICITY_HPP
