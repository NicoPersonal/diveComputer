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
    
    if (!std::filesystem::exists(filename)) {
        ErrorHandler::logError("SetPoints", "Setpoints file not found at " + filename + 
                               ". Using defaults.", ErrorSeverity::INFO);
        return false;
    }
    
    return ErrorHandler::tryFileOperation([&]() {
        std::ifstream file(filename, std::ios::binary);
        if (!file.is_open()) {
            throw std::ios_base::failure("Failed to open setpoints file for reading");
        }
        
        // Clear existing setpoints
        m_depths.clear();
        m_setPoints.clear();
        
        // Read number of setpoints
        size_t count = 0;
        file.read(reinterpret_cast<char*>(&count), sizeof(count));
        
        // Reserve space
        m_depths.reserve(count);
        m_setPoints.reserve(count);
        
        // Read each setpoint
        for (size_t i = 0; i < count; ++i) {
            double depth = 0.0;
            double setPoint = 0.0;
            
            file.read(reinterpret_cast<char*>(&depth), sizeof(depth));
            file.read(reinterpret_cast<char*>(&setPoint), sizeof(setPoint));
            
            if (file.fail()) {
                throw std::ios_base::failure("Error reading setpoint data");
            }
            
            // Add to vectors
            m_depths.push_back(depth);
            m_setPoints.push_back(setPoint);
        }
        
        file.close();
        
        // Sort setpoints
        sortSetPoints();
        
        ErrorHandler::logError("SetPoints", "Loaded " + std::to_string(count) + 
                             " setpoints successfully", ErrorSeverity::INFO);
    }, filename, "Error Loading Setpoints");
}

bool SetPoints::saveSetPointsToFile() {
    const std::string filename = getFilePath(SETPOINTS_FILE_NAME);
    
    return ErrorHandler::tryFileOperation([&]() {
        // Create directory if it doesn't exist
        std::filesystem::path filePath(filename);
        std::filesystem::create_directories(filePath.parent_path());
        
        std::ofstream file(filename, std::ios::binary | std::ios::trunc);
        if (!file.is_open()) {
            throw std::ios_base::failure("Failed to open file for writing: " + filename);
        }
        
        // Write number of setpoints
        size_t count = m_depths.size();
        file.write(reinterpret_cast<const char*>(&count), sizeof(count));
        
        // Write each setpoint
        for (size_t i = 0; i < count; ++i) {
            file.write(reinterpret_cast<const char*>(&m_depths[i]), sizeof(m_depths[i]));
            file.write(reinterpret_cast<const char*>(&m_setPoints[i]), sizeof(m_setPoints[i]));
            
            if (file.fail()) {
                throw std::ios_base::failure("Error writing setpoint data");
            }
        }
        
        file.close();
        
        ErrorHandler::logError("SetPoints", "Saved " + std::to_string(count) + 
                             " setpoints successfully", ErrorSeverity::INFO);
    }, filename, "Error Saving Setpoints");
}

} // namespace DiveComputer