#include "dive_plan.hpp"
#include <random>


namespace DiveComputer {

DivePlan::DivePlan(double depth, double time, diveMode mode, int diveNumber, std::vector<CompartmentPP> initialPressure)
{
    m_diveNumber = diveNumber;
    m_mode = mode;

    m_initialPressure = initialPressure;
    
    m_stopSteps.clear();
    m_stopSteps.addStopStep(depth, time);

    // Load available gases before building the dive plan
    loadAvailableGases();

    // Build the dive plan
    build();

    // Calculate the dive plan
    calculate();

    // Update gas consumption
    updateGasConsumption();
}

// Core methods
void DivePlan::loadAvailableGases() {
    m_gasAvailable.clear();
    
    for (const auto& gas : g_gasList.getGases()) {
        // Only add active gases
        if (gas.m_gasStatus == GasStatus::ACTIVE) {
            m_gasAvailable.emplace_back(gas);
        }
    }
    
    // Ensure we have at least one gas (air as default)
    if (m_gasAvailable.empty()) {
        Gas defaultGas(g_constants.m_oxygenInAir, 0.0, GasType::BOTTOM, GasStatus::ACTIVE);
        m_gasAvailable.emplace_back(defaultGas);
    }
}

void DivePlan::build(){
    clear();

    // Set mode
    stepMode activeMode = (m_mode == diveMode::CC) ? stepMode::CC : stepMode::OC;

    // Add initial surface step
    auto& surfaceStep = addStep(0, 0, 0, Phase::STOP, activeMode);
    surfaceStep.m_ppActual = m_initialPressure;

    if (m_stopSteps.nbOfStopSteps() == 0) return;

    // Sort and get deepest stop
    m_stopSteps.sortDescending();
    double maxDepth = m_stopSteps.m_stopSteps[0].m_depth;

    // Add descending phase and initial deep stop
    addStep(0.0, 0.0, 0, Phase::GAS_SWITCH, activeMode);
    addStep(0.0, maxDepth, maxDepth / g_parameters.m_maxDescentRate, Phase::DESCENDING, activeMode);
    addStep(maxDepth, maxDepth, m_stopSteps.m_stopSteps[0].m_time, Phase::STOP, activeMode);
    
    // Collect all stops in one pass
    std::set<double> allStops = { maxDepth, 0.0 };
    
    // Add planned stops
    for (int i = 1; i < m_stopSteps.nbOfStopSteps(); i++) {
        allStops.insert(m_stopSteps.m_stopSteps[i].m_depth);
    }
    
    // Add required intermediate stops at multiples of m_depthIncrement
    for (double depth = calculateFirstStopDepth(maxDepth); 
         depth >= g_parameters.m_lastStopDepth; 
         depth -= g_parameters.m_depthIncrement) {
        if (std::abs(depth - maxDepth) > 0.1) {  // Skip if already added deepest stop
            allStops.insert(depth);
        }
    }
    
    // Add last stop depth if needed
    allStops.insert(g_parameters.m_lastStopDepth);
    
    // Convert to vector and sort descending
    std::vector<double> ascentStops(allStops.begin(), allStops.end());
    std::sort(ascentStops.begin(), ascentStops.end(), std::greater<double>());
    
    // Build the profile
    processAscentStops(ascentStops);

    // Initialise the ppActual for Step 0
    m_diveProfile[0].m_ppActual = m_initialPressure;
}

void DivePlan::calculate() {
    ErrorHandler::tryOperation([this]() {
        // Guard against inconsistent state
        if (m_diveProfile.empty()) {
            throw std::runtime_error("Cannot calculate with empty dive profile");
        }

        m_firstDecoDepth = 0;

        // Update phase from first deco
        updateStepsPhaseFromFirstDeco();
        
        // Apply gases
        applyGases();

        // Initialise the gradient factor
        applyGF();

        // Calculate ppInertGas for all steps
        calculatePPInertGas();

        // Calculate ppInertGasMax for all steps
        calculatePPInertGasMax();


        // START LOGGING
        printPlan(m_diveProfile);
        int step = ;
        m_diveProfile[step].printStepDetails(step);

        // END LOGGING


        // Returns the first deco stop, required for applying the GF
        setFirstDecoDepth();

        // Apply the gradient factor to each step based on first deco stop determined
        applyGF();

        // Calculate the pp_max values for each step adjusted for the GF
        calculatePPInertGasMax();   

        // Update phase from first deco
        updateStepsPhaseFromFirstDeco();

        // Re-apply gases after the first deco is found as the maxPPo2 will have changed for deco steps
        applyGases();

        // Calculate deco steps
        calculateDecoSteps();

        // Update other variables
        updateStepsPhaseFromFirstDeco();
        updateVariables(100); // GF 100 for ceiling
        updateTimeProfile();
    }, "DivePlan::calculate", "Calculation Error");
}

void DivePlan::updateGasConsumption() {
    if (m_gasAvailable.empty() || m_diveProfile.empty()) {
        return;
    }
    
    // Reset consumption for all gases
    for (auto& gas : m_gasAvailable) {
        gas.m_consumption = 0.0;
    }
    
    // Calculate consumption for each step
    for (const auto& step : m_diveProfile) {
        // Find the matching gas for this step
        for (auto& gas : m_gasAvailable) {
            if (std::abs(gas.m_gas.m_o2Percent - step.m_o2Percent) < 0.1 && 
                std::abs(gas.m_gas.m_hePercent - step.m_hePercent) < 0.1) {
                // Add this step's consumption to the gas
                gas.m_consumption += step.m_stepConsumption;
                break;
            }
        }
    }
    
    // Calculate end pressure for each gas
    for (auto& gas : m_gasAvailable) {
        if (gas.m_nbTanks > 0 && gas.m_tankCapacity > 0) {
            gas.m_endPressure = gas.m_fillingPressure - (gas.m_consumption / (gas.m_nbTanks * gas.m_tankCapacity));
        } else {
            gas.m_endPressure = 0.0;
        }
    }
}

void DivePlan::defineMission() {
    // TODO: Implement
}

void DivePlan::setMaxTime() {       
    // TODO: Implement  
}

void DivePlan::optimiseDecoGas() {
    // TODO: Implement
}

double DivePlan::getTTSMax(){
    // TODO: Implement
    return 100.0;
}

double DivePlan::getTTS(){
    // TODO: Implement
    return 100.0;
}

double DivePlan::getTTSDelta() {
    // TODO: Implement
    return 9.5;
}

double DivePlan::getTP() {
    // TODO: Implement
    return 105.0;
}

// HELPER METHODS

void DivePlan::clear(){
    m_diveProfile.clear();
}

void DivePlan::sortGases(){
    // Sort gases by increasing O2 then increasing He
    std::sort(m_gasAvailable.begin(), m_gasAvailable.end(), [](const GasAvailable& a, const GasAvailable& b) {
        if (a.m_gas.m_o2Percent != b.m_gas.m_o2Percent) {
            return a.m_gas.m_o2Percent < b.m_gas.m_o2Percent;
        }
        return a.m_gas.m_hePercent < b.m_gas.m_hePercent;
    });
}

void DivePlan::applyGases() {
    if (m_gasAvailable.empty()) {
        return;
    }

    // Sort gases
    sortGases();

    // Delete all GAS_SWITCH steps
    std::vector<DiveStep> filteredSteps;
    for (const auto& step : m_diveProfile) {
        if (step.m_phase != Phase::GAS_SWITCH) {
            filteredSteps.push_back(step);
        }
    }
    m_diveProfile = filteredSteps;

    // Create a new profile vector to hold steps with gas switches
    std::vector<DiveStep> updatedProfile;
    updatedProfile.reserve(m_diveProfile.size() * 2); // Reserve space for potential new steps
    
    // Track previous step's gas to detect changes
    double prevO2Percent = g_constants.m_oxygenInAir;
    double prevHePercent = 0.0;
    stepMode prevMode = stepMode::CC;
    
    // Process each step and add gas switches in a single pass
    for (size_t i = 0; i < m_diveProfile.size(); ++i) {
        auto& step = m_diveProfile[i];
        const Gas* selectedGas = nullptr;
        double maxDepth = std::max(step.m_startDepth, step.m_endDepth);
            
        // Check for surface step
        if(std::abs(step.m_startDepth) < 0.1 && std::abs(step.m_endDepth) < 0.1) {
            Gas gas = Gas(); // Default to Air
            selectedGas = &gas;
            step.m_o2Percent = gas.m_o2Percent;
            step.m_hePercent = gas.m_hePercent;
        }
        else {
            // Determine the max ppO2 for this phase
            double maxppO2 = 0.0;
            switch(step.m_mode) {
                case stepMode::OC:
                    maxppO2 = g_parameters.m_PpO2Active;
                    break;
                case stepMode::BAILOUT:
                    maxppO2 = g_parameters.m_PpO2Active;
                    break;
                case stepMode::DECO:
                    maxppO2 = g_parameters.m_PpO2Deco;
                    break;
                case stepMode::CC:
                    maxppO2 = g_parameters.m_maxPpO2Diluent;
                    break;
                default:
                    maxppO2 = g_parameters.m_PpO2Active;
                    break;
            }
            // Find the gas with the smallest MOD that can be used at this depth    
            double smallestMOD = std::numeric_limits<double>::max();

            for (const auto& gas : m_gasAvailable) {
                double gasMOD = gas.m_gas.MOD(maxppO2);                

                // Check if the gas MOD is smaller than the smallest MOD and greater than or equal to the current depth
                if (gasMOD < smallestMOD && gasMOD >= maxDepth) {
                    smallestMOD = gasMOD;
                    selectedGas = &gas.m_gas;
                }
            }

            // If no gas available, use the first gas available (which has the lowest O2 content)
            if (selectedGas == nullptr) {
                // Try to use the first available gas as a default
                if (!m_gasAvailable.empty()) {
                    selectedGas = &m_gasAvailable[0].m_gas;
                } else {
                    // If no gas available, use a default air
                    static Gas defaultAir;
                    selectedGas = &defaultAir;
                }
            }

            step.m_pAmbMax = std::max(getPressureFromDepth(step.m_startDepth), getPressureFromDepth(step.m_endDepth));

            if(step.m_mode == stepMode::CC) {
                step.m_o2Percent = std::min(m_setPoints.getSetPointAtDepth(maxDepth, m_boosted) / step.m_pAmbMax * 100.0, 100.0);
                step.m_hePercent = (100 - step.m_o2Percent) * selectedGas->m_hePercent / (100 - selectedGas->m_o2Percent);
            }
            else{
                step.m_o2Percent = selectedGas->m_o2Percent;
                step.m_hePercent = selectedGas->m_hePercent;
            }
        }

        // Calculate N2 and other values for the step
        step.m_n2Percent = 100.0 - step.m_o2Percent - step.m_hePercent;
        step.m_pO2Max = (step.m_o2Percent / 100.0) * step.m_pAmbMax;

        // Check if we need to add a gas switch step
        if (i > 0 && !(step.m_mode == stepMode::CC && prevMode == stepMode::CC) &&
            (std::abs(step.m_o2Percent - prevO2Percent) > 0.1 || 
             std::abs(step.m_hePercent - prevHePercent) > 0.1)) {
            
            // Find the corresponding GasAvailable object
            for (auto& gasAvailable : m_gasAvailable) {
                if (&gasAvailable.m_gas == selectedGas) {
                    gasAvailable.m_switchDepth = step.m_startDepth;
                    gasAvailable.m_switchPpO2 = step.m_pAmbMax * step.m_o2Percent / 100.0;
                    break;
                }
            }

            // Create a gas switch step
            DiveStep gasSwitch = step;
            gasSwitch.m_endDepth = step.m_startDepth;
            gasSwitch.m_time = 0.01; // Minimal time
            gasSwitch.m_phase = Phase::GAS_SWITCH; 
            
            // Use the gas of the current step (the new gas being switched to)
            
            // Add the gas switch step to the updated profile
            updatedProfile.push_back(gasSwitch);
        }
        
        // Add the current step to the updated profile
        updatedProfile.push_back(step);
        
        // Store current step's gas info for the next iteration
        prevO2Percent = step.m_o2Percent;
        prevHePercent = step.m_hePercent;
        prevMode = step.m_mode;
    }
    
    // Replace the original profile with the updated one
    m_diveProfile = std::move(updatedProfile);

}

void DivePlan::calculateDecoSteps(){
    // TODO: Implement this

    for (int i = 0; i < nbOfSteps(); i++){
        if (m_diveProfile[i].m_phase == Phase::DECO){

            if(m_diveProfile[i].m_endDepth <= m_firstDecoDepth){
                m_diveProfile[i].m_time = (m_firstDecoDepth - m_diveProfile[i].m_endDepth)/ g_parameters.m_depthIncrement + 1;
            }
        }
    }
}

double DivePlan::calculateFirstStopDepth(double maxDepth){
    double firstStopDepth = std::ceil(maxDepth / g_parameters.m_depthIncrement) * g_parameters.m_depthIncrement;
    return (firstStopDepth > maxDepth) ? firstStopDepth - g_parameters.m_depthIncrement : firstStopDepth;
}

void DivePlan::processAscentStops(const std::vector<double>& ascentStops){
    stepMode ascentMode = (m_mode == diveMode::CC) ? (m_bailout ? stepMode::BAILOUT : stepMode::CC) : stepMode::OC;

    for (size_t i = 0; i < ascentStops.size() - 1; ++i) {
        double fromDepth = ascentStops[i];
        double toDepth = ascentStops[i+1];
        
        // Add ascending step
        double ascendTime = (fromDepth - toDepth) / g_parameters.m_maxAscentRate;
        addStep(fromDepth, toDepth, ascendTime, Phase::ASCENDING, ascentMode);
        
        // Check if this is a planned stop
        bool isPlannedStop = false;
        double stopTime = 0;
        
        for (const auto& stop : m_stopSteps.m_stopSteps) {
            if (std::abs(stop.m_depth - toDepth) < 0.1) {
                isPlannedStop = true;
                stopTime = stop.m_time;
                break;
            }
        }
        
        // Add appropriate step
        if (isPlannedStop) {
            addStep(toDepth, toDepth, stopTime, Phase::STOP, ascentMode);
            if (toDepth > 0) {
                addStep(toDepth, toDepth, 0, Phase::DECO, ascentMode);
            }
        } else if (toDepth > 0) {
            addStep(toDepth, toDepth, 0, Phase::DECO, ascentMode);
        } else {
            addStep(toDepth, toDepth, 0, Phase::STOP, ascentMode);
        }
    }
}

int DivePlan::nbOfSteps(){
    return m_diveProfile.size();
}

DiveStep& DivePlan::addStep(double start_depth, double end_depth, double time, Phase phase, stepMode mode){
    DiveStep step;
    step.m_startDepth = start_depth;
    step.m_endDepth = end_depth;
    step.m_time = time;
    step.m_phase = phase;
    step.m_mode = mode;
    m_diveProfile.push_back(step);
    return m_diveProfile.back();
}

DiveStep& DivePlan::insertStep(int index, double start_depth, double end_depth, double time, Phase phase, stepMode mode){
    DiveStep step;
    step.m_startDepth = start_depth;
    step.m_endDepth = end_depth;
    step.m_time = time;
    step.m_phase = phase;
    step.m_mode = mode;
    m_diveProfile.insert(m_diveProfile.begin() + index, step);
    return m_diveProfile[index];
}

void DivePlan::deleteStep(int index) {
    if (index >= 0 && index < static_cast<int>(m_diveProfile.size())) {
        m_diveProfile.erase(m_diveProfile.begin() + index);
    }
}

// Decompression methods

void DivePlan::calculatePPInertGas() {
    for (int i = 1; i < (int) m_diveProfile.size(); i++) {
        m_diveProfile[i].calculatePPInertGasForStep(m_diveProfile[i - 1], m_diveProfile[i].m_time);
    }
}

void DivePlan::calculatePPInertGasMax() {
    double lastRatioN2He = 1.0;

    for (int i = 1; i < (int) m_diveProfile.size(); i++) {
        m_diveProfile[i].calculatePPInertGasMaxForStep(lastRatioN2He);
    }
}

void DivePlan::applyGF() {
    for (int i = 1; i < (int) m_diveProfile.size(); i++) {
        m_diveProfile[i].m_gf = getGF(m_diveProfile[i].m_endDepth, m_firstDecoDepth);
    }
}

void DivePlan::setFirstDecoDepth() {
    int i = 1; // First Stop after the bottom phase
    bool breached = false;

    while (i < (int) m_diveProfile.size()) {
        if (m_diveProfile[i].getIfBreachingDecoLimits()) {
            std::cout << "Breached deco limits at depth " << m_diveProfile[i].m_endDepth << std::endl;
            breached = true;
            break;
        }
        i++;
    }

    if (!breached) {
        m_firstDecoDepth = 0.0;
    }
    else{
        double depth_breached = m_diveProfile[i].m_endDepth;

        for (int j = i - 1; j >= 0; j--) {
            if (m_diveProfile[j].m_endDepth > depth_breached) {
                m_firstDecoDepth = m_diveProfile[j].m_endDepth;
                break;
            }
        }
    }
}

// Update variable functions

void DivePlan::updateStepsPhaseFromFirstDeco(){
    stepMode decoMode = (m_mode == diveMode::CC && !m_bailout) ? stepMode::CC : stepMode::DECO;

    for (int i = 0; i < (int) m_diveProfile.size(); i++) {
        if (m_diveProfile[i].m_phase == Phase::DECO && m_diveProfile[i].m_startDepth <= m_firstDecoDepth) {
            int j = i;
            while (j < (int) m_diveProfile.size() && m_diveProfile[j].m_phase != Phase::STOP) {
                m_diveProfile[j].m_mode = decoMode;
                j += 1;
            }
        }
    }
}

void DivePlan::updatePpAmb(){
    for (int i = 0; i < nbOfSteps(); i++){
        m_diveProfile[i].updatePAmb();
    }
}

void DivePlan::updateCeiling(double GF){
    for (int i = 0; i < nbOfSteps(); i++){
        m_diveProfile[i].updateCeiling(GF);
    }
}

void DivePlan::updateOxygenToxicity(){
    for (int i = 1; i < nbOfSteps(); i++) {
        m_diveProfile[i].updateOxygenToxicity(&m_diveProfile[i - 1]);
    }
}

void DivePlan::updateConsumptions(){
    for (int i = 0; i < nbOfSteps(); i++) {
        m_diveProfile[i].updateConsumption();
    }
}

void DivePlan::updateGFSurface(){
    
    for (int i = 0; i < (int) m_diveProfile.size(); i++) {
        m_diveProfile[i].updateGFSurface(&m_diveProfile[nbOfSteps() - 1]);
    }
}

void DivePlan::updateRunTimes(){
    
    for (int i = 1; i < (int) m_diveProfile.size(); i++) {
        m_diveProfile[i].updateRunTime(&m_diveProfile[i - 1]);
    }
}

void DivePlan::updateVariables(double GF){
    updateStepsPhaseFromFirstDeco();

    for (int i = 0; i < nbOfSteps(); i++){
        m_diveProfile[i].updatePAmb();
        m_diveProfile[i].updateCeiling(GF);        
        m_diveProfile[i].updateConsumption();
        m_diveProfile[i].m_pO2Max = m_diveProfile[i].m_pAmbMax * m_diveProfile[i].m_o2Percent / 100.0;
        m_diveProfile[i].m_n2Percent = 100.0 - m_diveProfile[i].m_o2Percent - m_diveProfile[i].m_hePercent;
        m_diveProfile[i].updateGFSurface(&m_diveProfile[nbOfSteps() - 1]);
        m_diveProfile[i].updateDensity();
        m_diveProfile[i].updateEND();

        if (i > 0){
            m_diveProfile[i].updateOxygenToxicity(&m_diveProfile[i - 1]);
            m_diveProfile[i].updateRunTime(&m_diveProfile[i - 1]);
        }
        else{
            m_diveProfile[0].m_runTime = m_diveProfile[0].m_time;
        }
    }
}

void DivePlan::updateTimeProfile(){

    double time_increment = g_parameters.m_timeIncrementDeco;
    int total_time_steps = (int) (m_diveProfile[nbOfSteps() - 1].m_runTime - m_diveProfile[0].m_runTime) / time_increment;

    m_timeProfile.clear();
    m_timeProfile.resize(total_time_steps);

    int diveplan_index = 0;
    int timeplan_index = 0;
        
    double run_time = m_diveProfile[0].m_runTime + time_increment;
    double CNS_total_single_dive = 0;
    double CNS_total_multiple_dives = 0;
    double OTU_total = 0;

    for (diveplan_index = 1; diveplan_index < nbOfSteps(); diveplan_index++){
        double diveplan_start_time = m_diveProfile[diveplan_index].m_runTime - m_diveProfile[diveplan_index].m_time;
        double diveplan_end_time = m_diveProfile[diveplan_index].m_runTime;

        while (diveplan_start_time < run_time && run_time <= diveplan_end_time){
            m_timeProfile[timeplan_index] = m_diveProfile[diveplan_index];
            
            // Only adjusts the values which are dependant on time
            m_timeProfile[timeplan_index].m_time = time_increment;
            m_timeProfile[timeplan_index].m_runTime = run_time;

            m_timeProfile[timeplan_index].m_stepConsumption = m_timeProfile[timeplan_index].m_ambConsumptionAtDepth * time_increment;
            
            m_timeProfile[timeplan_index].m_cnsStepSingleDive = 
                (m_timeProfile[timeplan_index].m_cnsMaxMinSingleDive != 0) ? 100 * time_increment / m_timeProfile[timeplan_index].m_cnsMaxMinSingleDive : 0;
            CNS_total_single_dive += m_timeProfile[timeplan_index].m_cnsStepSingleDive;
            m_timeProfile[timeplan_index].m_cnsTotalSingleDive = CNS_total_single_dive;

            m_timeProfile[timeplan_index].m_cnsStepMultipleDives = 
                (m_timeProfile[timeplan_index].m_cnsMaxMinMultipleDives != 0) ? 100 * time_increment / m_timeProfile[timeplan_index].m_cnsMaxMinMultipleDives : 0;
            CNS_total_multiple_dives += m_timeProfile[timeplan_index].m_cnsStepMultipleDives;
            m_timeProfile[timeplan_index].m_cnsTotalMultipleDives = CNS_total_multiple_dives;

            m_timeProfile[timeplan_index].m_otuStep = time_increment * m_timeProfile[timeplan_index].m_otuPerMin;
            OTU_total += m_timeProfile[timeplan_index].m_otuStep;
            m_timeProfile[timeplan_index].m_otuTotal = OTU_total;

            double pp_time = run_time - (m_diveProfile[diveplan_index].m_runTime - m_diveProfile[diveplan_index].m_time);
            m_timeProfile[timeplan_index].calculatePPInertGasForStep(m_diveProfile[diveplan_index], pp_time);

            timeplan_index++;
            run_time += time_increment;
        }
    }

    for (int i = 0; i < total_time_steps; i++){
        m_timeProfile[i].updateGFSurface(&m_diveProfile[nbOfSteps() - 1]);
    }
}

// Print-to-terminal functions

void DivePlan::printPlan(std::vector<DiveStep> profile){
    printf("\nDIVE PROFILE\n\n");
 
    // Print the header
    printf("----------------------------------------------------------------------------------------------------------------------------------------------------\n");
    printf("|Step| Phase|    Depth   |  time /   run | Pamb / ppO2 |   GF  | GFSurf | O2 /  He /  N2  | SAC/ Amb /  Tot |  d  |    END (m)  |   CNS (%%)  | OTU |\n");
    printf("|  # |      |     (m)    |      (min)    |  max (bar)  |       |        |      (%%)        | (L/min)  /  (L) |(g/L)| non O2 / O2 | Dive | Day | min |\n");
    printf("----------------------------------------------------------------------------------------------------------------------------------------------------\n");

    for (int i = 0; i < nbOfSteps(); i++){
        printf("|%3i | ", i);
        if (profile[i].m_phase == Phase::DESCENDING)          printf("DESC | ");
        if (profile[i].m_phase == Phase::GAS_SWITCH)          printf("GAS  | ");
        if (profile[i].m_phase == Phase::STOP)                printf("STOP | ");
        if (profile[i].m_phase == Phase::MISSION)             printf("MISS | ");
        if (profile[i].m_phase == Phase::DECO)                printf("DECO | ");
        if (profile[i].m_phase == Phase::ASCENDING)           printf("ASC  | ");
        if (profile[i].m_phase == Phase::GROUPED_ASCENDING)   printf("ASC* | ");
        
        printf("%3.0f -> %3.0f | %5.1f / %5.1f | %4.1f / %3.2f | %4.0f  |  %4.0f  | %3.0f / %3.0f / %3.0f | %2.0f / %3.0f / %4.0f | %3.1f |   %3.0f / %3.0f |  %3.0f | %3.0f | %3.0f |\n", 
            profile[i].m_startDepth, profile[i].m_endDepth, 
            profile[i].m_time, profile[i].m_runTime, 
            profile[i].m_pAmbMax, profile[i].m_pO2Max,
            profile[i].m_gf, profile[i].m_gfSurface,
            profile[i].m_o2Percent, profile[i].m_hePercent, profile[i].m_n2Percent, 
            profile[i].m_sacRate, profile[i].m_ambConsumptionAtDepth, profile[i].m_stepConsumption,
            profile[i].m_gasDensity, profile[i].m_endWithoutO2, profile[i].m_endWithO2,
            profile[i].m_cnsTotalSingleDive, profile[i].m_cnsTotalMultipleDives, profile[i].m_otuTotal);
    }

    printf("----------------------------------------------------------------------------------------------------------------------------------------------------\n\n");
}

void DivePlan::printCompartmentDetails(int compartment){
    printf("| Step | Comp | Depth | P_amb |   GF  | pp_GF_n2 | pp_n2 | pp_GF_he | pp_he | pp_GF_inert | pp_inert |\n");

    for (int i = 0; i < nbOfSteps(); i++){
        m_diveProfile[i].printCompartmentDetails(i, compartment);
    }
}

void DivePlan::printGF(){
    for (int i = 0; i < nbOfSteps(); i++){
        printf("Step: %3i | Depth: %3.0f | GF: %3.0f%%\n", i, m_diveProfile[i].m_endDepth, m_diveProfile[i].m_gf);
    }
}

void DivePlan::printO2Exposure(){
    printf("----------------------------------------------------------------------------------------------------\n");
    printf("| Step | Time | ppO2_max |  CNS Single |  CNS%% Step |  CNS Daily |  CNS%% Step | OTU/min | OTU Step |\n");
    printf("----------------------------------------------------------------------------------------------------\n");

    for (int i = 0; i < nbOfSteps(); i++){
        printf("| %3i  |  %3.0f |   %4.2f   |      %3.0f    |    %5.1f   |     %3.0f    |   %5.1f    |  %4.1f   |    %5.1f |\n", 
            i, m_diveProfile[i].m_time, m_diveProfile[i].m_pO2Max, 
            m_diveProfile[i].m_cnsMaxMinSingleDive, m_diveProfile[i].m_cnsStepSingleDive,
            m_diveProfile[i].m_cnsMaxMinMultipleDives, m_diveProfile[i].m_cnsStepMultipleDives,
            m_diveProfile[i].m_otuPerMin, m_diveProfile[i].m_otuStep);
    }
    printf("----------------------------------------------------------------------------------------------------\n");
    printf(" Total                                      %5.1f                    %5.1f                   %5.1f\n",
        m_diveProfile[nbOfSteps() - 1].m_cnsTotalSingleDive, m_diveProfile[nbOfSteps() - 1].m_cnsTotalMultipleDives, 
        m_diveProfile[nbOfSteps() - 1].m_otuTotal);
}



} // namespace DiveComputer
