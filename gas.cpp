#include "gas.hpp"

namespace DiveComputer {

Gas::Gas() { // Defaults to Air
    m_o2Percent = g_constants.m_oxygenInAir;
    m_hePercent = 0.0;
    m_gasType = GasType::BOTTOM;
    m_gasStatus = GasStatus::ACTIVE;
    m_MOD = MOD(g_parameters.m_PpO2Active);
}

Gas::Gas(double o2Percent, double hePercent, GasType gasType, GasStatus gasStatus) {
    m_o2Percent = o2Percent;
    m_hePercent = hePercent;
    m_gasType = gasType;
    m_gasStatus = gasStatus;

    double maxppO2 = 0.0;
    if (gasType == GasType::BOTTOM) {
        maxppO2 = g_parameters.m_PpO2Active;
    } else if (gasType == GasType::DECO) {
        maxppO2 = g_parameters.m_PpO2Deco;
    } else if (gasType == GasType::DILUENT) {
        maxppO2 = g_parameters.m_maxPpO2Diluent;
    }

    m_MOD = MOD(maxppO2);
}

Gas Gas::bestGasForDepth(double depth, GasType gasType) {
    double maxppO2 = 0.0;
    if (gasType == GasType::BOTTOM) {
        maxppO2 = g_parameters.m_PpO2Active;
    } else if (gasType == GasType::DECO) {
        maxppO2 = g_parameters.m_PpO2Deco;
    } else if (gasType == GasType::DILUENT) {
        maxppO2 = g_parameters.m_maxPpO2Diluent;
    }

    // Define O2 and He content of the gas
    double o2Percent = 100.0 * (maxppO2 / getPressureFromDepth(depth));
    double hePercent = getOptimalHeContent(depth, o2Percent);
    
    Gas bestGas(o2Percent, hePercent, gasType, GasStatus::ACTIVE);
    return bestGas;
}

double Gas::MOD(double ppO2) const {
    return getDepthFromPressure(ppO2 / (m_o2Percent / 100.0));
}

double Gas::Density(double depth) const {
    double density = getPressureFromDepth(depth) * 
                   (g_constants.m_tempStp / (g_parameters.m_tempMin + g_constants.m_tempStp)) * 
                   (m_o2Percent / 100.0 * g_constants.m_o2Density + 
                    m_hePercent / 100.0 * g_constants.m_heDensity + 
                    (100 - m_o2Percent - m_hePercent) / 100.0 * g_constants.m_n2Density);

    return density;
}

double Gas::ENDWithoutO2(double depth) const {
    double END = (((100 - m_o2Percent - m_hePercent) / 100.0) / (1.0 - g_constants.m_oxygenInAir / 100.0) * getPressureFromDepth(depth) - 
                g_constants.m_atmPressureStp) * g_constants.m_meterPerBar;
    
    return std::max(END, 0.0);
}

double Gas::ENDWithO2(double depth) const {
    double END = ((100 - m_hePercent) / 100.0 * getPressureFromDepth(depth) - 
               g_constants.m_atmPressureStp) * g_constants.m_meterPerBar;

    return std::max(END, 0.0);
}

} // namespace DiveComputer