#ifndef COMPARTMENTS_HPP
#define COMPARTMENTS_HPP

#include "constants.hpp"
#include <vector>

namespace DiveComputer {

// Buhlman 16 + 1a compartments
const int NUM_COMPARTMENTS = 17;

// Buhlman parameters
class CompartmentParameters {
public:
    CompartmentParameters() = default;
    CompartmentParameters(double halfTimeN2, double aN2, double bN2, 
                          double halfTimeHe, double aHe, double bHe);

    double m_halfTimeN2{0.0};  // Half-time for N2 in minutes
    double m_aN2{0.0};         // A constant for N2
    double m_bN2{0.0};         // B constant for N2

    double m_halfTimeHe{0.0};  // Half-time for He in minutes
    double m_aHe{0.0};         // A constant for He
    double m_bHe{0.0};         // B constant for He
};

// Compartment partial pressure values
class CompartmentPP {
public:
    CompartmentPP() = default;
    CompartmentPP(double pN2, double pHe, double pInert);
    CompartmentPP(const CompartmentPP& other) 
        : m_pN2(other.m_pN2), m_pHe(other.m_pHe), m_pInert(other.m_pInert) {}

    // Assignment operator overload
    void setInitialPressureToAir();
    CompartmentPP& operator=(const CompartmentPP& other);

    double m_pN2{0.0};     // Max value for ppN2
    double m_pHe{0.0};     // Max value for ppHe
    double m_pInert{0.0};  // Max value for Inert Gas (based on gas composition)
};

extern std::vector<CompartmentPP> compartmentPPinitialAir;

} // namespace DiveComputer

#endif // TYPES_HPP
