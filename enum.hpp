#ifndef ENUM_HPP
#define ENUM_HPP

#include <iostream>

namespace DiveComputer {

enum class WindowPosition {
    CENTER,
    TOP_LEFT,
    TOP_RIGHT,
    BOTTOM_LEFT,
    BOTTOM_RIGHT
};

enum class Phase {
    DESCENDING,
    GAS_SWITCH,
    STOP,
    MISSION,
    DECO,
    ASCENDING,
    GROUPED_ASCENDING,
};

std::string getPhaseString(Phase phase);
std::string getPhaseIcon(Phase phase);
std::ostream& operator<<(std::ostream& os, const Phase& phase); 

enum class stepMode {
    CC,
    BAILOUT,
    OC,
    DECO,
};

std::string getStepModeString(stepMode mode);
std::string getStepModeIcon(stepMode mode);
std::ostream& operator<<(std::ostream& os, const stepMode& mode);

enum class diveMode {
    OC,
    CC
};

std::string getDiveModeString(diveMode mode);
std::ostream& operator<<(std::ostream& os, const diveMode& mode);

enum class GasType {
    BOTTOM,
    DECO,
    DILUENT,
};

std::string getGasTypeString(GasType type);
std::ostream& operator<<(std::ostream& os, const GasType& type);

enum class GasStatus {
    INACTIVE,
    ACTIVE,
};

std::string getGasStatusString(GasStatus status);
std::ostream& operator<<(std::ostream& os, const GasStatus& status);

} // namespace DiveComputer

#endif
