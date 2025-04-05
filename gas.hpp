#ifndef GAS_HPP
#define GAS_HPP

#include "constants.hpp"
#include "parameters.hpp"
#include "global.hpp"
#include "enum.hpp"

namespace DiveComputer {

 // Gas class
class Gas {
public:
    // Constructor
    Gas();
    Gas(double o2Percent, double hePercent, GasType gasType, GasStatus gasStatus);

    // Attributes
    double    m_o2Percent{0.0};
    double    m_hePercent{0.0};
    GasType   m_gasType{};
    GasStatus m_gasStatus{};
    double    m_MOD{0.0};

    // Methods
    static Gas bestGasForDepth(double depth, GasType gasType);
    double MOD(double ppO2) const;
    double Density(double depth) const;
    double ENDWithoutO2(double depth) const;
    double ENDWithO2(double depth) const;

};


} // namespace DiveComputer

#endif // GAS_HPP
