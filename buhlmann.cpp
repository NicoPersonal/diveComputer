#include "buhlmann.hpp"

namespace DiveComputer {

// Initialize global BuhlmannModel instance
BuhlmannModel g_buhlmannModel;

BuhlmannModel::BuhlmannModel() {
    // Initialize with Buhlmann parameters zh_l16C
    // Format: N2-1/2t, N2 a, N2 b, He-1/2t, He a, He b
    m_compartments = {{
        /*   Compartment 1  */ CompartmentParameters(4.0, 1.2599, 0.5240, 1.51, 1.7424, 0.4245),
        /*   Compartment 1a */ CompartmentParameters(5.0, 1.1696, 0.5578, 1.88, 1.6189, 0.4770),
        /*   Compartment 2  */ CompartmentParameters(8.0, 1.0000, 0.6514, 3.02, 1.3830, 0.5747),
        /*   Compartment 3  */ CompartmentParameters(12.5, 0.8618, 0.7222, 4.72, 1.1919, 0.6527),
        /*   Compartment 4  */ CompartmentParameters(18.5, 0.7562, 0.7825, 6.99, 1.0458, 0.7223),
        /*   Compartment 5  */ CompartmentParameters(27.0, 0.6200, 0.8126, 10.21, 0.9220, 0.7582),
        /*   Compartment 6  */ CompartmentParameters(38.3, 0.5043, 0.8434, 14.48, 0.8205, 0.7957),
        /*   Compartment 7  */ CompartmentParameters(54.3, 0.4410, 0.8693, 20.53, 0.7305, 0.8279),
        /*   Compartment 8  */ CompartmentParameters(77.0, 0.4000, 0.8910, 29.11, 0.6502, 0.8553),
        /*   Compartment 9  */ CompartmentParameters(109.0, 0.3750, 0.9092, 41.20, 0.5950, 0.8757),
        /*   Compartment 10 */ CompartmentParameters(146.0, 0.3500, 0.9222, 55.19, 0.5545, 0.8903),
        /*   Compartment 11 */ CompartmentParameters(187.0, 0.3295, 0.9319, 70.69, 0.5333, 0.8997),
        /*   Compartment 12 */ CompartmentParameters(239.0, 0.3065, 0.9403, 90.34, 0.5189, 0.9073),
        /*   Compartment 13 */ CompartmentParameters(305.0, 0.2835, 0.9477, 115.29, 0.5181, 0.9122),
        /*   Compartment 14 */ CompartmentParameters(390.0, 0.2610, 0.9544, 147.42, 0.5176, 0.9171),
        /*   Compartment 15 */ CompartmentParameters(498.0, 0.2480, 0.9602, 188.24, 0.5172, 0.9217),
        /*   Compartment 16 */ CompartmentParameters(635.0, 0.2327, 0.9653, 240.03, 0.5119, 0.9267)
    }};
}

} // namespace DiveComputer
