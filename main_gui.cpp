#include "main_gui.hpp"
#include "constants.hpp"
#include "dive_plan_dialog.hpp"

namespace DiveComputer {

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
    // Set window title
    QMainWindow::setWindowTitle("Dive Computer");
    
    // Create central widget with text label
    QWidget *centralWidget = new QWidget(this);
    QMainWindow::setCentralWidget(centralWidget);
    QVBoxLayout *layout = new QVBoxLayout(centralWidget);
    
    // Create logo label at the top
    logoLabel = new QLabel(centralWidget);
    QPixmap logo;

    // Try to load from file first (for customization)
    QString appDir = QCoreApplication::applicationDirPath();
    if (logo.load(appDir + QString::fromStdString(LOGO_FILE_NAME))) {
        // Successfully loaded custom logo
    } 
    // Fall back to embedded resource if file not found
    else {
        logo = QPixmap(":/images/" + QString::fromStdString(LOGO_FILE_NAME));
    }

    // Resize logo to fit nicely in the window (adjust as needed)
    if (!logo.isNull()) {
        logo = logo.scaledToWidth(300, Qt::SmoothTransformation);  
        logoLabel->setPixmap(logo);
        logoLabel->setAlignment(Qt::AlignCenter);
    } else {
        logoLabel->setText("Logo not found");
        logoLabel->setAlignment(Qt::AlignCenter);
    }
    
    layout->addWidget(logoLabel);

    // Create File menu
    QMenu *fileMenu = QMainWindow::menuBar()->addMenu("File");
    QAction *quitAction = fileMenu->addAction("Quit");
    quitAction->setShortcut(QKeySequence::Quit); // Cmd+Q on macOS
    // Connect to the application quit slot
    connect(quitAction, SIGNAL(triggered()), this, SLOT(quitApplication()));
    
    // Create Parameters action for macOS app menu
    QAction *parametersAction = new QAction("Parameters", this);
    parametersAction->setShortcut(QKeySequence::Preferences); // Cmd+, on macOS
    parametersAction->setMenuRole(QAction::PreferencesRole);
    this->addAction(parametersAction); // Add to main window's actions
    connect(parametersAction, SIGNAL(triggered()), this, SLOT(openParameterWindow()));
    
    // Create Gas Mixes action with Command+G shortcut
    QAction *gasMixesAction = new QAction("Gas Mixes", this);
    gasMixesAction->setShortcut(QKeySequence("Ctrl+G")); // Ctrl+G (macOS will show as Command+G)
    connect(gasMixesAction, SIGNAL(triggered()), this, SLOT(openGasListWindow()));

    // Create dive plan action with Command+D shortcut
    QAction *divePlanAction = new QAction("Create a dive plan", this);
    divePlanAction->setShortcut(QKeySequence("Ctrl+D")); // Ctrl+D (macOS will show as Command+D)
    connect(divePlanAction, SIGNAL(triggered()), this, SLOT(createDivePlan()));

    QAction *placeholderAction = new QAction("Placeholder", this);
    placeholderAction->setShortcut(QKeySequence("Ctrl+P")); // Ctrl+P (macOS will show as Command+P)
    connect(placeholderAction, SIGNAL(triggered()), this, SLOT(openPlaceholderWindow()));

    // Create Tools menu
    QMenu *toolsMenu = QMainWindow::menuBar()->addMenu("Tools");
    
    // Add actions to tools menu
    toolsMenu->addAction(parametersAction);
    toolsMenu->addAction(gasMixesAction);
    toolsMenu->addAction(divePlanAction);
    toolsMenu->addAction(placeholderAction);
    
    // Create a permanent Diveplanning menu (initially disabled)
    divePlanningMenu = QMainWindow::menuBar()->addMenu("Diveplanning");
    divePlanningMenu->setEnabled(false); // Initially disabled

    // Track all top-level windows for closing on quit
    childWindows = new QList<QWidget*>();

    // Use the common window sizing and positioning function
    setWindowSizeAndPosition(this, preferredWidth, preferredHeight, WindowPosition::CENTER);

    // Install event filter to handle window activation events
    qApp->installEventFilter(this);
}

MainWindow::~MainWindow() {
    // Clean up our window list
    delete childWindows;
}

void MainWindow::createDivePlan() {
    // Create and show the dive plan dialog
    DivePlanDialog dialog(this);
    
    if (dialog.exec() == QDialog::Accepted) {
        // Get values from dialog
        double depth = dialog.getDepth();
        double bottomTime = dialog.getBottomTime();
        diveMode mode = dialog.getDiveMode();
        
        // Create the dive plan window
        DivePlanWindow *divePlanWindow = new DivePlanWindow(depth, bottomTime, mode, this);
        divePlanWindow->setAttribute(Qt::WA_DeleteOnClose);
        
        // Track this window
        childWindows->append(divePlanWindow);
        
        // Remove from our list when it's closed
        connect(divePlanWindow, &QObject::destroyed, this, &MainWindow::handleDivePlanWindowDestroyed);
        
        // Activate with menu
        activateWindowWithMenu(divePlanWindow);
    }
}

void MainWindow::quitApplication() {
    // Close all child windows we've created
    QList<QWidget*> windowsCopy = *childWindows; // Make a copy since closing will modify the list
    for (QWidget* window : windowsCopy) {
        if (window && window->isVisible()) {
            window->close();
        }
    }
    
    // Now close the main window, which will exit the application
    close();
}

bool MainWindow::eventFilter(QObject* obj, QEvent* event) {
    if (event->type() == QEvent::WindowActivate) {
        QWidget* activeWindow = QApplication::activeWindow();
        DivePlanWindow* divePlanWindow = qobject_cast<DivePlanWindow*>(activeWindow);
        
        // Directly update the menu state based on the active window
        if (divePlanWindow) {
            // Enable menu and set it up for this window
            divePlanningMenu->setEnabled(true);
            activeDivePlanWindow = divePlanWindow;
            divePlanWindow->setDivePlanningMenu(divePlanningMenu);
        } else {
            // Disable menu when not on a dive plan window
            divePlanningMenu->setEnabled(false);
            activeDivePlanWindow = nullptr;
        }
    }
    
    return QMainWindow::eventFilter(obj, event);
}

void MainWindow::activateWindowWithMenu(DivePlanWindow* window) {
    // Store reference to active window
    activeDivePlanWindow = window;
    
    // Enable menu and configure it for this window
    divePlanningMenu->setEnabled(true);
    window->setDivePlanningMenu(divePlanningMenu);
    
    // Show the window
    window->show();
    window->raise();
    window->activateWindow();
}

void MainWindow::openGasListWindow() {
    openWindow<GasListWindow>(&gasListWindow);
}

void MainWindow::openParameterWindow() {
    openWindow<ParameterWindow>(&parameterWindow);
}

void MainWindow::openPlaceholderWindow() {
    openWindow<PlaceholderWindow>(&placeholderWindow);
}

void MainWindow::handleWindowDestroyed() {
    QObject* obj = sender();
    if (obj) {
        childWindows->removeOne(static_cast<QWidget*>(obj));
        
        // We can't directly set the pointer to nullptr here
        // Will need to manually check which window was destroyed
        if (gasListWindow == obj) gasListWindow = nullptr;
        else if (parameterWindow == obj) parameterWindow = nullptr;
        else if (placeholderWindow == obj) placeholderWindow = nullptr;
    }
}

void MainWindow::handleDivePlanWindowDestroyed() {
    // Get the sender (the window being destroyed)
    DivePlanWindow* divePlanWindow = qobject_cast<DivePlanWindow*>(sender());
    if (!divePlanWindow) return;
    
    childWindows->removeOne(divePlanWindow);
    
    // If this was the active dive plan window, clear our reference
    if (activeDivePlanWindow == divePlanWindow) {
        activeDivePlanWindow = nullptr;
        
        // Remove the menu
        if (divePlanningMenu) {
            menuBar()->removeAction(divePlanningMenu->menuAction());
            divePlanningMenu = nullptr;
        }
    }
}

} // namespace DiveComputer