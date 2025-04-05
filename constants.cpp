#include "constants.hpp"

namespace DiveComputer {

// Initialize global constants instance
Constants g_constants;

Constants::Constants() {
    // Initialize with default values
    calculateDerivedConstants();
}

void Constants::calculateDerivedConstants() {
    
    // Calculate derived constants based on primary constants
    m_barPerMeter = m_waterDensity * m_gravitation / 100000.0;
    m_meterPerBar = 1.0 / m_barPerMeter;
}

} // namespace DiveComputer
