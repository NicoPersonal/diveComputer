#include "oxygen_toxicity.hpp"
#include <iostream>

namespace DiveComputer {

// Initialize global OxygenToxicity instance
OxygenToxicity g_oxygenToxicity;

O2Exposure::O2Exposure(double ppO2Start, double ppO2End, 
                     double aCNSMaxMinSingleDive, double bCNSMaxMinSingleDive,
                     double aCNSMaxMinMultipleDives, double bCNSMaxMinMultipleDives)
    : m_ppO2Start(ppO2Start)
    , m_ppO2End(ppO2End)
    , m_aCNSMaxMinSingleDive(aCNSMaxMinSingleDive)
    , m_bCNSMaxMinSingleDive(bCNSMaxMinSingleDive)
    , m_aCNSMaxMinMultipleDives(aCNSMaxMinMultipleDives)
    , m_bCNSMaxMinMultipleDives(bCNSMaxMinMultipleDives)
{
}

OxygenToxicity::OxygenToxicity() {
    // Initialize NOAA table linearized values
    // Format: ppO2_start, ppO2_end, CNS_single a & b, CNS_multiple a & b
    m_o2ExposureParameters = {{
        O2Exposure(0.6, 0.7, -1500, 1620, -1500, 1620),
        O2Exposure(0.7, 0.8, -1200, 1410, -1200, 1410),
        O2Exposure(0.8, 0.9, -900, 1170, -900, 1170),
        O2Exposure(0.9, 1.1, -600, 900, -450, 765),
        O2Exposure(1.1, 1.5, -300, 570, -225, 517.5),
        O2Exposure(1.5, 1.65, -750, 1245, -300, 630)
    }};
}

double OxygenToxicity::getOTUPerMin(double ppO2Ambient) const {
    const double exponent = 0.833;
    
    return (ppO2Ambient >= 0.5) ? std::pow((ppO2Ambient - 0.5) / 0.5, exponent) : 0.0;
}

double OxygenToxicity::getCNSMaxMin(double ppO2Ambient, bool singleDive) const {
    double a = 0.0, b = 0.0;

    if (ppO2Ambient >= m_o2ExposureParameters[0].m_ppO2Start) {
        if (ppO2Ambient > m_o2ExposureParameters[NUM_O2_EXPOSURE_PARAMETERS - 1].m_ppO2End) {
            std::cout << "WARNING: ppO2 is greater than the highest ppO2 value in the NOAA table for oxygen toxicity" << std::endl;
            a = 0.0;
            b = 100000.0;
        } else {
            for (int i = 0; i < NUM_O2_EXPOSURE_PARAMETERS; i++) {
                if ((ppO2Ambient >= m_o2ExposureParameters[i].m_ppO2Start) && 
                    (ppO2Ambient <= m_o2ExposureParameters[i].m_ppO2End)) {
                    if (singleDive) {
                        a = m_o2ExposureParameters[i].m_aCNSMaxMinSingleDive;
                        b = m_o2ExposureParameters[i].m_bCNSMaxMinSingleDive;
                    } else {
                        a = m_o2ExposureParameters[i].m_aCNSMaxMinMultipleDives;
                        b = m_o2ExposureParameters[i].m_bCNSMaxMinMultipleDives;
                    }
                    break;
                }
            }
        }
    }

    return a * ppO2Ambient + b;
}

} // namespace DiveComputer
