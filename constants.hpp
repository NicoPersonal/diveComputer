#ifndef CONSTANTS_HPP
#define CONSTANTS_HPP

#include <vector>

namespace DiveComputer {

class Constants {
public:
    Constants();
    ~Constants() = default;

    void calculateDerivedConstants();

    const double m_atmPressureStp{1.01325};  // in hPa = bar. STP conditions used for gas density
    const double m_tempStp{273.15};         // in K. Standard Temperature and Pressure
    const double m_waterDensity{1023.6};    // in kg/m^3
    const double m_gravitation{9.81};       // in m.s^(-2)
    const double m_oxygenInAir{21.0};       // 21% oxygen in air, balance Nitrogen
    const double m_pH2O{0.0627};            // in bars | Resp. Quotient | Buhlman 1.0 |Schreiner 0.8->0,0493 bar | US Navy 0.9->0,0567 bar
    const double m_o2Density{1.429};        // g/L at STP (Standard Temperature and Pressure)
    const double m_heDensity{0.1786};       // g/L at STP
    const double m_n2Density{1.2506};       // g/L at STP

    // Derived constants
    double m_barPerMeter{0.0};        // Calculated from water density and gravitation
    double m_meterPerBar{0.0};        // Calculated as 1 / m_barPerMeter

};

// Global instance - this will be defined in constants.cpp
extern Constants g_constants;

} // namespace DiveComputer

#endif // CONSTANTS_HPP
