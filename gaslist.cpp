#include "gaslist.hpp"

namespace DiveComputer {

// Define the global GasList instance
GasList g_gasList;

GasList::GasList() {
    // Ensure app info is set before any path operations
    ensureAppInfoSet();
    loadGaslistFromFile();
}


void GasList::addGas(double o2Percent, double hePercent, GasType gasType, GasStatus gasStatus) {
    Gas gas(o2Percent, hePercent, gasType, gasStatus);
    gases.push_back(gas);
}

void GasList::editGas(int index, double o2Percent, double hePercent, GasType gasType, GasStatus gasStatus) {
    gases[index] = Gas(o2Percent, hePercent, gasType, gasStatus);
}

void GasList::deleteGas(int index) {
    gases.erase(gases.begin() + index);
}

void GasList::clearGaslist() {
    gases.clear();
}

bool GasList::loadGaslistFromFile() {
    const std::string filename = getFilePath(GASLIST_FILE_NAME);
    
    std::cout << "Trying to load gas list from: " << filename << std::endl;
    
    // Try to load gas list from file if it exists
    if (std::filesystem::exists(filename)) {
        std::cout << "Gas list file exists, loading it..." << std::endl;
        
        std::ifstream file(filename, std::ios::binary);
        if (!file.is_open()) {
            std::cerr << "Failed to open gas list file for reading." << std::endl;
            return false;
        } else {
            try {
                // Clear the list, determine the number of gases and reserve space for them
                gases.clear();
                size_t gasCount = 0;
                file.read(reinterpret_cast<char*>(&gasCount), sizeof(gasCount));
                gases.reserve(gasCount);
                
                // Read each gas
                for (size_t i = 0; i < gasCount; ++i) {
                    double o2Percent = 0.0;
                    double hePercent = 0.0;
                    GasType gasType = GasType::BOTTOM;
                    GasStatus gasStatus = GasStatus::ACTIVE;
                    
                    file.read(reinterpret_cast<char*>(&o2Percent), sizeof(o2Percent));
                    file.read(reinterpret_cast<char*>(&hePercent), sizeof(hePercent));
                    file.read(reinterpret_cast<char*>(&gasType), sizeof(gasType));
                    file.read(reinterpret_cast<char*>(&gasStatus), sizeof(gasStatus));
                    // Add the gas to our list
                    addGas(o2Percent, hePercent, gasType, gasStatus);
                }
                
                file.close();
                std::cout << "Gas list loaded successfully. Loaded " << gasCount << " gases." << std::endl;
                return true;
            }
            catch (const std::exception& e) {
                std::cerr << "Exception while reading gas list: " << e.what() << std::endl;
                file.close();
                return false;
            }
        }
    } else {
        std::cout << "Gas list file does not exist at " << filename << ". Using default values." << std::endl;
        
        // Since file doesn't exist, let's add a default gas (21% O2)
        if (gases.empty()) {
            addGas(21.0, 0.0, GasType::BOTTOM, GasStatus::ACTIVE);
        }
        
        // Save the current list to establish the file
        saveGaslistToFile();
        
        return false;
    }
}

bool GasList::saveGaslistToFile() {
    const std::string filename = getFilePath(GASLIST_FILE_NAME);
    
    std::cout << "Saving gas list to: " << filename << std::endl;
    
    // Create directories if they don't exist
    std::filesystem::path filePath(filename);
    std::filesystem::create_directories(filePath.parent_path());
    
    std::ofstream file(filename, std::ios::binary | std::ios::trunc);
    if (!file.is_open()) {
        std::cerr << "Failed to open file for writing: " << filename << std::endl;
        return false;
    }

    // First write the number of gases
    size_t gasCount = gases.size();
    file.write(reinterpret_cast<const char*>(&gasCount), sizeof(gasCount));
    
    // Then write each gas
    for (const Gas& gas : gases) {
        file.write(reinterpret_cast<const char*>(&gas.m_o2Percent), sizeof(gas.m_o2Percent));
        file.write(reinterpret_cast<const char*>(&gas.m_hePercent), sizeof(gas.m_hePercent));
        file.write(reinterpret_cast<const char*>(&gas.m_gasType), sizeof(gas.m_gasType));
        file.write(reinterpret_cast<const char*>(&gas.m_gasStatus), sizeof(gas.m_gasStatus));
    }

    file.close();
    
    // Verify the file was created
    if (std::filesystem::exists(filename)) {
        std::cout << "Gas list saved successfully to " << filename << ". File size: " 
                  << std::filesystem::file_size(filename) << " bytes" << std::endl;
        return true;
    } else {
        std::cerr << "Error: File does not exist after save operation!" << std::endl;
        return false;
    }
}

void GasList::print() {
    for (const Gas& gas : gases) {
        std::cout << "Gas: " << gas.m_o2Percent << "%, " << gas.m_hePercent << "%" << std::endl;
    }
}

} // namespace DiveComputer