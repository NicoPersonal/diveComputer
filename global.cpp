#include "global.hpp"

namespace DiveComputer {

// Ensure app info is properly set for path resolution
void ensureAppInfoSet() {
    // Set the organization and application name if not already set
    // These MUST be set before the first call to QStandardPaths
    if (QCoreApplication::organizationName().isEmpty()) {
        QCoreApplication::setOrganizationName("DiveComputer");
    }
    if (QCoreApplication::applicationName().isEmpty()) {
        QCoreApplication::setApplicationName("DiveComputer");
    }
}

std::string getFilePath(const std::string& filename) {
    // Ensure app info is set before getting the path
    ensureAppInfoSet();
    
    // Get application data location
    QString dataLocation = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    
    // Print the location for debugging
    std::cout << "Using AppDataLocation: " << dataLocation.toStdString() << std::endl;
    
    // Create directory if it doesn't exist
    QDir dir(dataLocation);
    if (!dir.exists()) {
        bool created = dir.mkpath(".");
        std::cout << "Created directory: " << (created ? "success" : "failed") << std::endl;
    }
    
    // Get full path with filename
    QString fullPath = dir.filePath(QString::fromStdString(filename));
    
    return fullPath.toStdString();
}

void setWindowSizeAndPosition(QWidget* window, int preferredWidth, int preferredHeight, WindowPosition position) {
    
    int margin = 10;

    // Get the screen resolution using the newer QScreen approach
    QScreen *screen = QApplication::primaryScreen();
    QRect screenGeometry = screen->availableGeometry();
    
    // Calculate appropriate window size (constrained by screen size)
    int windowWidth = std::min(screenGeometry.width() - margin, preferredWidth);
    int windowHeight = std::min(screenGeometry.height() - margin, preferredHeight);
    window->resize(windowWidth, windowHeight);
    
    // Calculate window position based on requested position
    int x, y;
    
    switch (position) {
        case WindowPosition::CENTER:
            x = (screenGeometry.width() - window->width()) / 2;
            y = (screenGeometry.height() - window->height()) / 2;
            break;
            
        case WindowPosition::TOP_LEFT:
            x = screenGeometry.left();
            y = screenGeometry.top();
            break;
            
        case WindowPosition::TOP_RIGHT:
            x = screenGeometry.right() - window->width();
            y = screenGeometry.top();
            break;
            
        case WindowPosition::BOTTOM_LEFT:
            x = screenGeometry.left();
            y = screenGeometry.bottom() - window->height();
            break;
            
        case WindowPosition::BOTTOM_RIGHT:
            x = screenGeometry.right() - window->width();
            y = screenGeometry.bottom() - window->height();
            break;
    }
    
    // Move the window to the calculated position
    window->move(x, y);
}

double getDepthFromPressure(double pressure) {
    return (pressure - g_constants.m_atmPressureStp) * g_constants.m_meterPerBar;
}

double getPressureFromDepth(double depth) {
    return g_constants.m_atmPressureStp + (g_constants.m_barPerMeter * depth);
}

double getOptimalHeContent(double depth, double o2Content) {
    double pAmbient = getPressureFromDepth(depth);
    double pAmbientNED = getPressureFromDepth(g_parameters.m_defaultEnd);
 
    double n2Content = 100.0 * ((!g_parameters.m_defaultO2Narcotic) ? 
        ((1.0 - g_constants.m_oxygenInAir / 100.0) * pAmbientNED / pAmbient) : 
        ((pAmbientNED / pAmbient) - o2Content / 100.0));
    
    n2Content = std::max(n2Content, 0.0);
    n2Content = std::min(n2Content, 100.0 - o2Content);

    double heContent = 100.0 - o2Content - n2Content;
    heContent = (heContent > static_cast<int>(heContent)) ? 
        static_cast<int>(heContent) + 1 : static_cast<int>(heContent);

    return heContent;
}

double getSchreinerEquation(double p0, double halfTime, double pAmbStartDepth, double pAmbEndDepth, double time, double inertPercent) {
    double piAmb = pAmbStartDepth;
    double pi = (piAmb - g_constants.m_pH2O) * inertPercent / 100.0;
    double k = log(2) / halfTime;
    double r = (time == 0) ? 0 : (pAmbEndDepth - piAmb) / time * inertPercent / 100.0;
 
    return pi + r * (time - 1/k) - (pi - p0 - r/k) * exp(-k * time);
}

double getGF(double depth, double firstDecoDepth) {
    double gf;

    if (depth > firstDecoDepth) {
        gf = g_parameters.m_gf[0];
    } else {
        gf = std::min(g_parameters.m_gf[1], 
                g_parameters.m_gf[0] + (g_parameters.m_gf[1] - g_parameters.m_gf[0]) * 
                (depth - firstDecoDepth) / (g_parameters.m_lastStopDepth - firstDecoDepth));
    }

    return gf;
}

double getDouble(const std::string& prompt) {
    double result;
    bool validInput = false;
    
    while (!validInput) {
        std::cout << prompt;
        
        if (std::cin >> result) {
            // Input is valid
            validInput = true;
            
            // Clear the rest of the line
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        } else {
            // Input is not valid
            std::cout << "Invalid input. Please enter a valid decimal number." << std::endl;
            
            // Clear the error state
            std::cin.clear();
            
            // Clear the input buffer
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        }
    }
    
    return result;
}

}