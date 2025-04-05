#include "enum.hpp"


namespace DiveComputer {

// Overload the << operator for the enums

std::string getPhaseString(Phase phase) {
    std::string phaseString;
    
    switch(phase) {
        case Phase::DESCENDING:
            phaseString = "Descending";
            break;
        case Phase::STOP:
            phaseString = "Stop";
            break;
        case Phase::DECO:
            phaseString = "Deco";
            break;
        case Phase::MISSION:
            phaseString = "Mission";
            break;
        case Phase::GAS_SWITCH:
            phaseString = "Gas switch";
            break;
        case Phase::ASCENDING:
            phaseString = "Ascending";
            break;
        case Phase::GROUPED_ASCENDING:
            phaseString = "Grouped ascending";
            break;
    }
    return phaseString;
}

std::string getPhaseIcon(Phase phase) {
    std::string phaseString;

    switch (phase) {
        case Phase::ASCENDING:
            phaseString = "↑";  // Vertical arrow pointing up
            break;
        case Phase::GROUPED_ASCENDING:
            phaseString = "⇑";  // Double vertical arrow pointing up
            break;
        case Phase::DESCENDING:
            phaseString = "↓";  // Vertical arrow pointing down
            break;
        case Phase::STOP:
            phaseString = "→";  // Horizontal arrow pointing right
            break;
        case Phase::MISSION:
            phaseString = "⇒";  // Double horizontal arrow pointing right
            break;
        case Phase::DECO:
            phaseString = "🕒";  // Clock symbol
            break;
        case Phase::GAS_SWITCH:
            phaseString = "⇄";  // Two horizontal arrows pointing in opposite directions
            break;
        default:
            phaseString = getPhaseString(phase);
            break;
    }
    
    return phaseString;
}

std::ostream& operator<<(std::ostream& os, const Phase& phase) {
    std::string phaseString = getPhaseString(phase);
    os << phaseString;
    return os;
}

std::string getStepModeIcon(stepMode mode) {
    std::string stepModeString;
    
    switch(mode) {
        case stepMode::CC:
            stepModeString = "🔄";
            break;
        case stepMode::OC:
            stepModeString = "🏊";
            break;
        case stepMode::BAILOUT:
            stepModeString = "⚠️";
            break;
        case stepMode::DECO:
            stepModeString = "🧘";
            break;
        default:
            stepModeString = getStepModeString(mode);
            break;
    }

    return stepModeString;
}

std::string getStepModeString(stepMode mode) {
    std::string stepModeString;
    switch(mode) {
        case stepMode::CC:
            stepModeString = "CC";
            break;  
        case stepMode::BAILOUT:
            stepModeString = "Bailout";
            break;
        case stepMode::OC:
            stepModeString = "OC";
            break;
        case stepMode::DECO:
            stepModeString = "Deco";
            break;
    }
    return stepModeString;
}

std::ostream& operator<<(std::ostream& os, const stepMode& mode) {
    std::string stepModeString = getStepModeString(mode);
    os << stepModeString;
    return os;
}

std::string getGasTypeString(GasType type) {
    std::string gasTypeString;
    switch(type) {
        case GasType::BOTTOM:
            gasTypeString = "Bottom";
            break;
        case GasType::DECO:
            gasTypeString = "Deco";
            break;
        case GasType::DILUENT:
            gasTypeString = "Diluent";
            break;
    }
    return gasTypeString;
}

std::ostream& operator<<(std::ostream& os, const GasType& type) {
    std::string gasTypeString = getGasTypeString(type);
    os << gasTypeString;
    return os;
}

std::string getDiveModeString(diveMode mode) {
    std::string diveModeString;
    switch(mode) {
        case diveMode::OC:
            diveModeString = "OC";
            break;
        case diveMode::CC:  
            diveModeString = "CC";
            break;
    }
    return diveModeString;
}   

std::ostream& operator<<(std::ostream& os, const diveMode& mode) {
    std::string diveModeString = getDiveModeString(mode);
    os << diveModeString;
    return os;
}

std::string getGasStatusString(GasStatus status) {
    std::string gasStatusString;
    switch(status) {
        case GasStatus::ACTIVE:
            gasStatusString = "Active";
            break;
        case GasStatus::INACTIVE:
            gasStatusString = "Inactive";
            break;
    }
    return gasStatusString;
}

std::ostream& operator<<(std::ostream& os, const GasStatus& status) {
    std::string gasStatusString = getGasStatusString(status);
    os << gasStatusString;
    return os;
}



} // namespace DiveComputer 