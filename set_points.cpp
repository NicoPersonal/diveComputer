#include "set_points.hpp"

namespace DiveComputer {

SetPoints::SetPoints() {
    // Ensure app info is set before any path operations
    ensureAppInfoSet();

    // Try to load from file
    if (!loadSetPointsFromFile()) {
        m_setPoints.clear();
        setToDefault();
    }
    sortSetPoints();
}

void SetPoints::setToDefault() {
    addSetPoint(1000.0, 1.3);
    addSetPoint(40.0, 1.4);
    addSetPoint(21.0, 1.5);
    addSetPoint(6.0, 1.6);
}

// Sort the setpoints by decreasing depth, then decreasing setpoint
void SetPoints::sortSetPoints() {
    // Sort the depths and setpoints in descending order
    std::vector<double> sortedDepths;
    std::vector<double> sortedSetPoints;

    // Create a vector of pairs of depths and setpoints
    std::vector<std::pair<double, double>> depthSetpoints;
    for (size_t i = 0; i < m_depths.size(); ++i) {
        depthSetpoints.push_back(std::make_pair(m_depths[i], m_setPoints[i]));
    }   

    // Sort the vector of pairs by decreasing depth, then decreasing setpoint
    std::sort(depthSetpoints.begin(), depthSetpoints.end(), [](const std::pair<double, double>& a, const std::pair<double, double>& b) {
        if (a.first == b.first) {
            return a.second > b.second;
        }
        return a.first > b.first;
    });

    // Extract the sorted depths and setpoints
    for (const auto& pair : depthSetpoints) {
        sortedDepths.push_back(pair.first);
        sortedSetPoints.push_back(pair.second);
    }

    // Update the member variables with the sorted values
    m_depths = sortedDepths;
    m_setPoints = sortedSetPoints;  
}

// Find the setpoint at a given depth
double SetPoints::getSetPointAtDepth(double depth, bool boosted) {
    // First, ensure setpoints are sorted by decreasing depth, then decreasing setpoint
    sortSetPoints();
    
    // If no setpoints defined, return a default value (Diluent max PpO2)
    if (m_depths.empty()) {
        return g_parameters.m_maxPpO2Diluent;
    }
    
    // Case A: If depth is greater than or equal to the deepest setpoint
    if ((depth >= m_depths[0]) || !boosted){
        return m_setPoints[0]; // Return the setpoint for deepest depth
    }
    
    // Case B: If depth is less than the shallowest setpoint
    if (depth < m_depths[m_depths.size() - 1]) {
        return m_setPoints[m_depths.size() - 1]; // Return the shallowest setpoint
    }
    
    // Case C: Depth is between deepest and shallowest setpoints
    // Find the setpoint with the depth immediately greater than the target depth
    for (size_t i = 0; i < m_depths.size() - 1; i++) {
        if (depth < m_depths[i] && depth >= m_depths[i + 1]) {
            return m_setPoints[i]; // Return the setpoint with depth immediately greater
        }
    }
    
    // Fallback (should not reach here if all cases are covered properly)
    return m_setPoints[0];
}

void SetPoints::addSetPoint(double depth, double setpoint) {
    m_depths.push_back(depth);
    m_setPoints.push_back(setpoint);
    sortSetPoints();
}

void SetPoints::removeSetPoint(size_t index) {
    if (index < m_depths.size()) {
        m_depths.erase(m_depths.begin() + index);
        m_setPoints.erase(m_setPoints.begin() + index);
    }
}

bool SetPoints::loadSetPointsFromFile() {
    const std::string filename = getFilePath(SETPOINTS_FILE_NAME);
    
    std::cout << "Trying to load setpoints from: " << filename << std::endl;
    
    // Clear current setpoints
    m_depths.clear();
    m_setPoints.clear();
    
    // Try to load setpoints from file if it exists
    if (std::filesystem::exists(filename)) {
        std::cout << "Setpoints file exists, loading it..." << std::endl;
        
        std::ifstream file(filename, std::ios::binary);
        if (!file.is_open()) {
            std::cerr << "Failed to open setpoints file for reading." << std::endl;
            return false;
        } else {
            try {
                // Read number of setpoints
                size_t count;
                file.read(reinterpret_cast<char*>(&count), sizeof(count));
                
                // Read setpoints
                for (size_t i = 0; i < count; ++i) {
                    double depth, setpoint;
                    file.read(reinterpret_cast<char*>(&depth), sizeof(depth));
                    file.read(reinterpret_cast<char*>(&setpoint), sizeof(setpoint));
                    
                    m_depths.push_back(depth);
                    m_setPoints.push_back(setpoint);
                }
                
                file.close();
                std::cout << "Setpoints loaded successfully. Count: " << count << std::endl;
                
                // Add a default setpoint if none were loaded
                if (m_depths.empty()) {
                    setToDefault();
                }
                
                return true;
            }
            catch (const std::exception& e) {
                std::cerr << "Exception while reading setpoints: " << e.what() << std::endl;
                file.close();
                return false;
            }
        }
    } else {
        std::cout << "Setpoints file does not exist at " << filename << ". Using default values." << std::endl;
        
        // Add default setpoint
        setToDefault();
        
        // Save default values to establish the file
        saveSetPointsToFile();
        
        return false;
    }
}



bool SetPoints::saveSetPointsToFile() const {
    const std::string filename = getFilePath(SETPOINTS_FILE_NAME);
    
    std::cout << "Saving setpoints to: " << filename << std::endl;
    
    // Create directories if they don't exist
    std::filesystem::path filePath(filename);
    std::filesystem::create_directories(filePath.parent_path());
    
    std::ofstream file(filename, std::ios::binary | std::ios::trunc);
    if (!file.is_open()) {
        std::cerr << "Failed to open file for writing: " << filename << std::endl;
        return false;
    }

    // Write number of setpoints
    size_t count = nbOfSetPoints();
    file.write(reinterpret_cast<const char*>(&count), sizeof(count));
    
    // Write each setpoint
    for (size_t i = 0; i < count; ++i) {
        file.write(reinterpret_cast<const char*>(&m_depths[i]), sizeof(m_depths[i]));
        file.write(reinterpret_cast<const char*>(&m_setPoints[i]), sizeof(m_setPoints[i]));
    }

    file.close();
    
    // Verify the file was created
    if (std::filesystem::exists(filename)) {
        std::cout << "Setpoints saved successfully to " << filename << ". File size: " 
                  << std::filesystem::file_size(filename) << " bytes" << std::endl;
        return true;
    } else {
        std::cerr << "Error: File does not exist after save operation!" << std::endl;
        return false;
    }
}

} // namespace DiveComputer