
#include "qtheaders.hpp"
#include "main_gui.hpp"

int main(int argc, char *argv[]) {
    // Initialize QT Application
    QApplication app(argc, argv);
    
    // Set organization and application name BEFORE any QStandardPaths calls
    // These affect where QStandardPaths looks for application data
    QCoreApplication::setOrganizationName("DiveComputer");
    QCoreApplication::setApplicationName("DiveComputer");

    // Create and show main window
    DiveComputer::MainWindow mainWindow;
    mainWindow.show();
    
    // Run the application event loop
    return app.exec();
}