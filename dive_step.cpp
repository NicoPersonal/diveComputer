#include "dive_step.hpp"


namespace DiveComputer {

std::ostream& operator<<(std::ostream& os, const DiveStep& step)
{
    os << std::fixed << std::setprecision(2)
       << std::setw(8) << step.m_startDepth << " m → "
       << std::setw(8) << step.m_endDepth << " m  | "
       << std::setw(10) << step.m_time << " min | "
       << std::left << std::setw(12) << step.m_phase << " | "
       << std::setw(8) << step.m_mode;
    return os;
}

double DiveStep::getGFSurface(DiveStep *stepSurface){
    double GF_surface = 0;
    
    for (int j = 0; j < NUM_COMPARTMENTS; j++){
        double GF_surface_n2 = 0, GF_surface_he = 0, GF_surface_inert = 0;

        GF_surface_n2    = (m_ppActual[j].m_pN2    - g_parameters.m_atmPressure) / (stepSurface->m_ppMax[j].m_pN2    - g_parameters.m_atmPressure) * 100;
        GF_surface_he    = (m_ppActual[j].m_pHe    - g_parameters.m_atmPressure) / (stepSurface->m_ppMax[j].m_pHe    - g_parameters.m_atmPressure) * 100;
        GF_surface_inert = (m_ppActual[j].m_pInert - g_parameters.m_atmPressure) / (stepSurface->m_ppMax[j].m_pInert - g_parameters.m_atmPressure) * 100;

        GF_surface = std::max(GF_surface, GF_surface_n2);
        GF_surface = std::max(GF_surface, GF_surface_he);
        GF_surface = std::max(GF_surface, GF_surface_inert);
    }

    return GF_surface;
}

double DiveStep::getCeiling(double GF){
    double ceiling_n2 = 0, ceiling_he = 0, ceiling_inert = 0;

    for (int j = 0; j < NUM_COMPARTMENTS; j++){
        double p_amb_min_n2, p_amb_min_he, p_amb_min_inert;
        
        // N2
        double a_n2 = g_buhlmannModel.m_compartments[j].m_aN2;
        double b_n2 = g_buhlmannModel.m_compartments[j].m_bN2;
        p_amb_min_n2 = (m_ppActual[j].m_pN2 - a_n2 * GF / 100) /  (1 + (1 / b_n2 - 1) * GF / 100);
        ceiling_n2 = std::max(ceiling_n2, getDepthFromPressure(p_amb_min_n2));
        
        // He
        double a_he = g_buhlmannModel.m_compartments[j].m_aHe;
        double b_he = g_buhlmannModel.m_compartments[j].m_bHe;
        p_amb_min_he = (m_ppActual[j].m_pHe - a_he * GF / 100) /  (1 + (1 / b_he - 1) * GF / 100);
        ceiling_he = std::max(ceiling_he, getDepthFromPressure(p_amb_min_he));
        
            
        // For total inert gas: adjusted by the proportion of N2 over (N2 + He)
        // If only O2 is breathed, then no condition on total inert gas. Max out P_Inert_Max
            
        double ratio_n2_he = 1;
        double total_inert_percent = m_n2Percent + m_hePercent;
            
        if (total_inert_percent != 0){
            ratio_n2_he = m_n2Percent / total_inert_percent;
        }
            
        double a_inert = g_buhlmannModel.m_compartments[j].m_aN2 * ratio_n2_he + g_buhlmannModel.m_compartments[j].m_aHe * (1 - ratio_n2_he);
        double b_inert = g_buhlmannModel.m_compartments[j].m_bN2 * ratio_n2_he + g_buhlmannModel.m_compartments[j].m_bHe * (1 - ratio_n2_he);
        p_amb_min_inert = (m_ppActual[j].m_pInert - a_inert * GF / 100) /  (1 + (1 / b_inert - 1) * GF / 100);
        ceiling_inert = std::max(ceiling_inert, getDepthFromPressure(p_amb_min_inert));
    }

    return std::max(ceiling_n2, std::max(ceiling_he, ceiling_inert));
}

void DiveStep::calculatePPInertGasForStep(DiveStep& previousStep, double time) {
    double pAmbStart = m_pAmbStartDepth;
    double pAmbEnd = m_pAmbEndDepth;
    double inertPercentN2 = m_n2Percent;
    double inertPercentHe = m_hePercent;

    for (int j = 0; j < NUM_COMPARTMENTS; j++) {
        double p0N2 = 0.0;
        double p0He = 0.0;
            
        p0N2 = previousStep.m_ppActual[j].m_pN2;
        p0He = previousStep.m_ppActual[j].m_pHe; 

        double halfTimeN2 = g_buhlmannModel.getCompartment(j).m_halfTimeN2;
        double halfTimeHe = g_buhlmannModel.getCompartment(j).m_halfTimeHe;

        double pN2 = getSchreinerEquation(p0N2, halfTimeN2, pAmbStart, pAmbEnd, time, inertPercentN2);
        double pHe = getSchreinerEquation(p0He, halfTimeHe, pAmbStart, pAmbEnd, time, inertPercentHe);
        double pInert = pN2 + pHe;

        CompartmentPP pp(pN2, pHe, pInert);
        m_ppActual[j] = pp;
    }
}

void DiveStep::calculatePPInertGasMaxForStep(double& lastRatioN2He) {
    // calculated on the lowest P_amb during that phase
    double pAmb = std::min(m_pAmbEndDepth, m_pAmbStartDepth);
    double gf = m_gf;

    for (int j = 0; j < NUM_COMPARTMENTS; j++) {
        // N2
        double aN2 = g_buhlmannModel.getCompartment(j).m_aN2;
        double bN2 = g_buhlmannModel.getCompartment(j).m_bN2;
        double pMaxN2 = aN2 + pAmb / bN2;

        // He
        double aHe = g_buhlmannModel.getCompartment(j).m_aHe;
        double bHe = g_buhlmannModel.getCompartment(j).m_bHe;
        double pMaxHe = aHe + pAmb / bHe;
            
        // Create PPMax object for this compartment
        CompartmentPP ppMax(pMaxN2, pMaxHe, 0.0); // pInert will be calculated below
        m_ppMax[j] = ppMax;
            
        // Calculate the pp_max adjusted for the GF
        double pMaxAdjustedN2 = pAmb + (pMaxN2 - pAmb) * gf / 100.0;
        double pMaxAdjustedHe = pAmb + (pMaxHe - pAmb) * gf / 100.0;

        // For total inert gas: adjusted by the proportion of N2 over (N2 + He)
        double ratioN2He = lastRatioN2He;
        double totalInertPercent = m_n2Percent + m_hePercent;
            
        if (totalInertPercent != 0.0) {
            ratioN2He = m_n2Percent / totalInertPercent;
            lastRatioN2He = ratioN2He;
        }
            
        double aInert = g_buhlmannModel.getCompartment(j).m_aN2 * ratioN2He + 
                      g_buhlmannModel.getCompartment(j).m_aHe * (1.0 - ratioN2He);
        double bInert = g_buhlmannModel.getCompartment(j).m_bN2 * ratioN2He + 
                      g_buhlmannModel.getCompartment(j).m_bHe * (1.0 - ratioN2He);
        double pMaxInert = aInert + pAmb / bInert;
            
        // Update the PPMax object with the inert value
        CompartmentPP ppMaxWithInert(pMaxN2, pMaxHe, pMaxInert);
        m_ppMax[j] = ppMaxWithInert;
            
        double pMaxAdjustedInert = pAmb + (pMaxInert - pAmb) * gf / 100.0;
            
        // Create PPMaxAdjustedGF object for this compartment
        CompartmentPP ppMaxAdjustedGF(pMaxAdjustedN2, pMaxAdjustedHe, pMaxAdjustedInert);
        m_ppMaxAdjustedGF[j] = ppMaxAdjustedGF;
    }
}

bool DiveStep::getIfBreachingDecoLimits() {
    bool breached = false;
    
    for (int j = 0; j < NUM_COMPARTMENTS; j++){
         breached = (m_ppActual[j].m_pN2 > m_ppMaxAdjustedGF[j].m_pN2) ||
                   (m_ppActual[j].m_pHe > m_ppMaxAdjustedGF[j].m_pHe) ||
                   (m_ppActual[j].m_pInert > m_ppMaxAdjustedGF[j].m_pInert);            

        if (breached) break;
    }

    return breached;
}

// update functions

void DiveStep::updatePAmb(){
    m_pAmbStartDepth = getPressureFromDepth(m_startDepth);
    m_pAmbEndDepth = getPressureFromDepth(m_endDepth);
    m_pAmbMax = std::max(m_pAmbStartDepth, m_pAmbEndDepth);
}

void DiveStep::updateCeiling(double GF){
    m_ceiling = getCeiling(GF);
}

void DiveStep::updateOxygenToxicity(DiveStep *previousStep){

    m_cnsMaxMinSingleDive = g_oxygenToxicity.getCNSMaxMin(m_pO2Max, true);
    if(m_cnsMaxMinSingleDive != 0.0) m_cnsStepSingleDive = m_time / m_cnsMaxMinSingleDive * 100;
    else m_cnsStepSingleDive = 0.0;
    m_cnsTotalSingleDive = previousStep->m_cnsTotalSingleDive + m_cnsStepSingleDive;

    m_cnsMaxMinMultipleDives = g_oxygenToxicity.getCNSMaxMin(m_pO2Max, false);
    if(m_cnsMaxMinMultipleDives != 0.0) m_cnsStepMultipleDives = m_time / m_cnsMaxMinMultipleDives * 100;
    else m_cnsStepMultipleDives = 0.0;
    m_cnsTotalMultipleDives = previousStep->m_cnsTotalMultipleDives + m_cnsStepMultipleDives;
        
    m_otuPerMin = g_oxygenToxicity.getOTUPerMin(m_pO2Max);
    m_otuStep = m_time * m_otuPerMin;
    m_otuTotal = previousStep->m_otuTotal + m_otuStep;

}

void DiveStep::updateDensity(){
    Gas tempGas(m_o2Percent, m_hePercent, GasType::BOTTOM, GasStatus::ACTIVE);
    m_gasDensity = tempGas.Density(std::max(m_startDepth, m_endDepth));
}

void DiveStep::updateEND(){
    Gas tempGas(m_o2Percent, m_hePercent, GasType::BOTTOM, GasStatus::ACTIVE);
    m_endWithoutO2 = tempGas.ENDWithoutO2(std::max(m_startDepth, m_endDepth));
    m_endWithO2 = tempGas.ENDWithO2(std::max(m_startDepth, m_endDepth));
}

void DiveStep::updateConsumption(){
        m_sacRate = 
            (m_mode == stepMode::CC) ? 0 : 
            (m_mode == stepMode::BAILOUT) ? g_parameters.m_sacBailout : 
            (m_mode == stepMode::OC) ? g_parameters.m_sacBottom : 
            g_parameters.m_sacDeco;

        m_ambConsumptionAtDepth = m_sacRate * 
            (getPressureFromDepth(m_startDepth) + getPressureFromDepth(m_endDepth)) / 2.0;
        
        m_stepConsumption = m_time * m_ambConsumptionAtDepth;
}

void DiveStep::updateGFSurface(DiveStep *stepSurface){

    m_gfSurface = getGFSurface(stepSurface);

}

void DiveStep::updateRunTime(DiveStep *previousStep){
    m_runTime = previousStep->m_runTime + m_time;
}

// Print to terminal functions

void DiveStep::printStepDetails(const int step) const {
    std::cout << "| Step | Comp | Depth | Depth | P_amb |   GF  | pp_GF_n2 | pp_n2 | pp_GF_he | pp_he | pp_GF_inert | pp_inert |" << std::endl;
    
    for (int j = 0; j < NUM_COMPARTMENTS; j++) {
        std::cout << "|  " << std::setw(3) << step << " |  " 
                  << std::setw(3) << j + 1 << " |  " 
                  << std::fixed << std::setprecision(0)
                  << std::setw(3) << m_startDepth << "  | " 
                  << std::fixed << std::setprecision(0)
                  << std::setw(3) << m_endDepth << "  | " 
                  << std::fixed << std::setprecision(2)
                  << std::setw(5) << m_pAmbStartDepth << " | " 
                  << std::fixed << std::setprecision(0)
                  << std::setw(4) << m_gf << "  |     " 
                  << std::fixed << std::setprecision(2)
                  << std::setw(5) << m_ppMaxAdjustedGF[j].m_pN2 << "| " 
                  << std::setw(5) << m_ppActual[j].m_pN2 << " |     " 
                  << std::setw(5) << m_ppMaxAdjustedGF[j].m_pHe << "| " 
                  << std::setw(5) << m_ppActual[j].m_pHe << " |        " 
                  << std::setw(5) << m_ppMaxAdjustedGF[j].m_pInert << "| " 
                  << std::setw(5) << m_ppActual[j].m_pInert << "    |" 
                  << std::setw(5) << m_o2Percent << "   /   " 
                  << std::setw(5) << m_hePercent << std::endl;
    }
}

void DiveStep::printCompartmentDetails(const int step, const int compartment) const {
    std::cout << "|  " << std::setw(3) << step << " |  " 
              << std::setw(3) << compartment << " |  " 
              << std::fixed << std::setprecision(0)
              << std::setw(3) << m_endDepth << "  | " 
              << std::fixed << std::setprecision(2)
              << std::setw(5) << m_pAmbStartDepth << " | " 
              << std::fixed << std::setprecision(0)
              << std::setw(4) << m_gf << "  |     " 
              << std::fixed << std::setprecision(2)
              << std::setw(5) << m_ppMaxAdjustedGF[compartment].m_pN2 << "| " 
              << std::setw(5) << m_ppActual[compartment].m_pN2 << " |     " 
              << std::setw(5) << m_ppMaxAdjustedGF[compartment].m_pHe << "| " 
              << std::setw(5) << m_ppActual[compartment].m_pHe << " |        " 
              << std::setw(5) << m_ppMaxAdjustedGF[compartment].m_pInert << "| " 
              << std::setw(5) << m_ppActual[compartment].m_pInert << "    |" 
              << std::setw(5) << m_o2Percent << "   /   " 
              << std::setw(5) << m_hePercent << std::endl;
}

}
