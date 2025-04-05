#include "compartments.hpp"

namespace DiveComputer {

CompartmentParameters::CompartmentParameters(double halfTimeN2, double aN2, double bN2, 
                                             double halfTimeHe, double aHe, double bHe)
    : m_halfTimeN2(halfTimeN2)
    , m_aN2(aN2)
    , m_bN2(bN2)
    , m_halfTimeHe(halfTimeHe)
    , m_aHe(aHe)
    , m_bHe(bHe)
{
}

CompartmentPP::CompartmentPP(double pN2, double pHe, double pInert)
    : m_pN2(pN2)
    , m_pHe(pHe)
    , m_pInert(pInert)
{
}

CompartmentPP& CompartmentPP::operator=(const CompartmentPP& other) {
    if (this != &other) {
        m_pN2 = other.m_pN2;
        m_pHe = other.m_pHe;
        m_pInert = other.m_pInert;
    }
    return *this;
}

const CompartmentPP compartmentAir(
    (g_constants.m_atmPressureStp - g_constants.m_pH2O) * (1.0 - g_constants.m_oxygenInAir / 100.0), 
    0.0, 
    (g_constants.m_atmPressureStp - g_constants.m_pH2O) * (1.0 - g_constants.m_oxygenInAir / 100.0)
);

std::vector<CompartmentPP> compartmentPPinitialAir(NUM_COMPARTMENTS, compartmentAir);

} // namespace DiveComputer