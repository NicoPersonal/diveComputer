#include "parameters.hpp"

namespace DiveComputer {

// Initialize global parameters instance
Parameters g_parameters;

Parameters::Parameters() {
    // Set to Delfult
    setToDefault();

    // Ensure app info is set before any path operations
    ensureAppInfoSet();
    loadParametersFromFile();
}

void Parameters::setToDefault() {
    // Default values stay the same
    m_gf[0] = 30.0;
    m_gf[1] = 80.0;
    m_atmPressure = 1.01325;
    m_tempMin = 20.0;
    m_defaultEnd = 24.0;
    m_defaultO2Narcotic = false;
    m_maxAscentRate = 9.0;
    m_maxDescentRate = 20.0;
    m_sacBottom = 18.0;
    m_sacBailout = 25.0;
    m_sacDeco = 15.0;
    m_o2CostPerL = 0.03;
    m_heCostPerL = 0.10;
    m_bestMixDepthBuffer = 5.0;
    m_PpO2Active = 1.40;
    m_PpO2Deco = 1.65;
    m_maxPpO2Diluent = 1.10;
    m_warningPpO2Low = 0.16;
    m_warningCnsMax = 80.0;
    m_warningOtuMax = 300.0;
    m_warningGasDensity = 6.2;
    m_depthIncrement = 3.0;
    m_lastStopDepth = 5.0;
    m_timeIncrementDeco = 1.0;
    m_timeIncrementMaxTime = 1.0;
    m_noFlyPressure = 0.7;
    m_noFlyGf = 50.0;
    m_noFlyTimeIncrement = 30.0;
}

bool Parameters::loadParametersFromFile() {
    const std::string filename = getFilePath(PARAMETERS_FILE_NAME);
    
    std::cout << "Trying to load parameters from: " << filename << std::endl;
    
    // Try to load parameters from file if it exists
    if (std::filesystem::exists(filename)) {
        std::cout << "Parameters file exists, loading it..." << std::endl;
        
        std::ifstream file(filename, std::ios::binary);
        if (!file.is_open()) {
            std::cerr << "Failed to open parameters file for reading." << std::endl;
            return false;
        } else {
            try {
                // Read parameters directly here instead of using ParametersStorage
                file.read(reinterpret_cast<char*>(&m_gf), sizeof(m_gf));
                file.read(reinterpret_cast<char*>(&m_atmPressure), sizeof(m_atmPressure));
                file.read(reinterpret_cast<char*>(&m_tempMin), sizeof(m_tempMin));
                file.read(reinterpret_cast<char*>(&m_defaultEnd), sizeof(m_defaultEnd));
                file.read(reinterpret_cast<char*>(&m_defaultO2Narcotic), sizeof(m_defaultO2Narcotic));
                file.read(reinterpret_cast<char*>(&m_maxAscentRate), sizeof(m_maxAscentRate));
                file.read(reinterpret_cast<char*>(&m_maxDescentRate), sizeof(m_maxDescentRate));
                file.read(reinterpret_cast<char*>(&m_sacBottom), sizeof(m_sacBottom));
                file.read(reinterpret_cast<char*>(&m_sacBailout), sizeof(m_sacBailout));
                file.read(reinterpret_cast<char*>(&m_sacDeco), sizeof(m_sacDeco));
                file.read(reinterpret_cast<char*>(&m_o2CostPerL), sizeof(m_o2CostPerL));
                file.read(reinterpret_cast<char*>(&m_heCostPerL), sizeof(m_heCostPerL));
                file.read(reinterpret_cast<char*>(&m_bestMixDepthBuffer), sizeof(m_bestMixDepthBuffer));
                file.read(reinterpret_cast<char*>(&m_PpO2Active), sizeof(m_PpO2Active));
                file.read(reinterpret_cast<char*>(&m_PpO2Deco), sizeof(m_PpO2Deco));
                file.read(reinterpret_cast<char*>(&m_maxPpO2Diluent), sizeof(m_maxPpO2Diluent));
                file.read(reinterpret_cast<char*>(&m_warningPpO2Low), sizeof(m_warningPpO2Low));
                file.read(reinterpret_cast<char*>(&m_warningCnsMax), sizeof(m_warningCnsMax));
                file.read(reinterpret_cast<char*>(&m_warningOtuMax), sizeof(m_warningOtuMax));
                file.read(reinterpret_cast<char*>(&m_warningGasDensity), sizeof(m_warningGasDensity));
                file.read(reinterpret_cast<char*>(&m_depthIncrement), sizeof(m_depthIncrement));
                file.read(reinterpret_cast<char*>(&m_lastStopDepth), sizeof(m_lastStopDepth));
                file.read(reinterpret_cast<char*>(&m_timeIncrementDeco), sizeof(m_timeIncrementDeco));
                file.read(reinterpret_cast<char*>(&m_timeIncrementMaxTime), sizeof(m_timeIncrementMaxTime));
                file.read(reinterpret_cast<char*>(&m_noFlyPressure), sizeof(m_noFlyPressure));
                file.read(reinterpret_cast<char*>(&m_noFlyGf), sizeof(m_noFlyGf));
                file.read(reinterpret_cast<char*>(&m_noFlyTimeIncrement), sizeof(m_noFlyTimeIncrement));
                
                file.close();
                std::cout << "Parameters loaded successfully." << std::endl;
                return true;
            }
            catch (const std::exception& e) {
                std::cerr << "Exception while reading parameters: " << e.what() << std::endl;
                file.close();
                return false;
            }
        }
    } else {
        std::cout << "Parameters file does not exist at " << filename << ". Using default values." << std::endl;
        
        // Since file doesn't exist, let's try saving default values to establish the file
        saveParametersToFile();
        
        return false;
    }
}

bool Parameters::saveParametersToFile() {
    const std::string filename = getFilePath(PARAMETERS_FILE_NAME);
    
    std::cout << "Saving parameters to: " << filename << std::endl;
    
    // Create directories if they don't exist
    std::filesystem::path filePath(filename);
    std::filesystem::create_directories(filePath.parent_path());
    
    std::ofstream file(filename, std::ios::binary | std::ios::trunc);
    if (!file.is_open()) {
        std::cerr << "Failed to open file for writing: " << filename << std::endl;
        return false;
    }

    // Write parameters directly to file
    file.write(reinterpret_cast<const char*>(&g_parameters.m_gf), sizeof(g_parameters.m_gf));
    file.write(reinterpret_cast<const char*>(&g_parameters.m_atmPressure), sizeof(g_parameters.m_atmPressure));
    file.write(reinterpret_cast<const char*>(&g_parameters.m_tempMin), sizeof(g_parameters.m_tempMin));
    file.write(reinterpret_cast<const char*>(&g_parameters.m_defaultEnd), sizeof(g_parameters.m_defaultEnd));
    file.write(reinterpret_cast<const char*>(&g_parameters.m_defaultO2Narcotic), sizeof(g_parameters.m_defaultO2Narcotic));
    file.write(reinterpret_cast<const char*>(&g_parameters.m_maxAscentRate), sizeof(g_parameters.m_maxAscentRate));
    file.write(reinterpret_cast<const char*>(&g_parameters.m_maxDescentRate), sizeof(g_parameters.m_maxDescentRate));
    file.write(reinterpret_cast<const char*>(&g_parameters.m_sacBottom), sizeof(g_parameters.m_sacBottom));
    file.write(reinterpret_cast<const char*>(&g_parameters.m_sacBailout), sizeof(g_parameters.m_sacBailout));
    file.write(reinterpret_cast<const char*>(&g_parameters.m_sacDeco), sizeof(g_parameters.m_sacDeco));
    file.write(reinterpret_cast<const char*>(&g_parameters.m_o2CostPerL), sizeof(g_parameters.m_o2CostPerL));
    file.write(reinterpret_cast<const char*>(&g_parameters.m_heCostPerL), sizeof(g_parameters.m_heCostPerL));
    file.write(reinterpret_cast<const char*>(&g_parameters.m_bestMixDepthBuffer), sizeof(g_parameters.m_bestMixDepthBuffer));
    file.write(reinterpret_cast<const char*>(&g_parameters.m_PpO2Active), sizeof(g_parameters.m_PpO2Active));
    file.write(reinterpret_cast<const char*>(&g_parameters.m_PpO2Deco), sizeof(g_parameters.m_PpO2Deco));
    file.write(reinterpret_cast<const char*>(&g_parameters.m_maxPpO2Diluent), sizeof(g_parameters.m_maxPpO2Diluent));
    file.write(reinterpret_cast<const char*>(&g_parameters.m_warningPpO2Low), sizeof(g_parameters.m_warningPpO2Low));
    file.write(reinterpret_cast<const char*>(&g_parameters.m_warningCnsMax), sizeof(g_parameters.m_warningCnsMax));
    file.write(reinterpret_cast<const char*>(&g_parameters.m_warningOtuMax), sizeof(g_parameters.m_warningOtuMax));
    file.write(reinterpret_cast<const char*>(&g_parameters.m_warningGasDensity), sizeof(g_parameters.m_warningGasDensity));
    file.write(reinterpret_cast<const char*>(&g_parameters.m_depthIncrement), sizeof(g_parameters.m_depthIncrement));
    file.write(reinterpret_cast<const char*>(&g_parameters.m_lastStopDepth), sizeof(g_parameters.m_lastStopDepth));
    file.write(reinterpret_cast<const char*>(&g_parameters.m_timeIncrementDeco), sizeof(g_parameters.m_timeIncrementDeco));
    file.write(reinterpret_cast<const char*>(&g_parameters.m_timeIncrementMaxTime), sizeof(g_parameters.m_timeIncrementMaxTime));
    file.write(reinterpret_cast<const char*>(&g_parameters.m_noFlyPressure), sizeof(g_parameters.m_noFlyPressure));
    file.write(reinterpret_cast<const char*>(&g_parameters.m_noFlyGf), sizeof(g_parameters.m_noFlyGf));
    file.write(reinterpret_cast<const char*>(&g_parameters.m_noFlyTimeIncrement), sizeof(g_parameters.m_noFlyTimeIncrement));

    file.close();
    
    // Verify the file was created
    if (std::filesystem::exists(filename)) {
        std::cout << "Parameters saved successfully to " << filename << ". File size: " 
                  << std::filesystem::file_size(filename) << " bytes" << std::endl;
        return true;
    } else {
        std::cerr << "Error: File does not exist after save operation!" << std::endl;
        return false;
    }
}

} // namespace DiveComputer