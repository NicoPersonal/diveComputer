#ifndef BUHLMANN_HPP
#define BUHLMANN_HPP

#include "compartments.hpp"
#include <array>

namespace DiveComputer {

// Buhlmann decompression model class
class BuhlmannModel {
public:
    BuhlmannModel();
    ~BuhlmannModel() = default;

    // Getter for compartments
    const CompartmentParameters& getCompartment(int index) const { return m_compartments[index]; }

    // Initialise with Buhlmann parameters zh_l16C
    std::array<CompartmentParameters, NUM_COMPARTMENTS> m_compartments;
};

// Global instance - will be defined in buhlmann.cpp
extern BuhlmannModel g_buhlmannModel;

} // namespace DiveComputer

#endif // BUHLMANN_HPP
