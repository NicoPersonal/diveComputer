#include "dive_plan_gui.hpp"
#include "main_gui.hpp"
#include "constants.hpp"
#include "dive_plan_dialog.hpp"
#include "ui_utils.hpp"

namespace DiveComputer {

// External globals
extern Parameters g_parameters;

DivePlanWindow::DivePlanWindow(double depth, double bottomTime, diveMode mode, QWidget *parent) 
    : QMainWindow(parent), 
      m_mainWindow(qobject_cast<MainWindow*>(parent)),
      leftPanelSplitter(nullptr),
      topWidgetsSplitter(nullptr),
      verticalSplitter(nullptr),
      m_tableDirty(false),
      m_isUpdating(false),
      m_gasesColumnsInitialized(false),  // Make sure to initialize
      m_totalGasesWidth(0)              // Make sure to initialize
{
    // Set window title
    setWindowTitle("Dive Plan");
    
    // Create initial dive plan
    m_divePlan = std::make_unique<DivePlan>(depth, bottomTime, mode, 1, compartmentPPinitialAir);
    
    // Load setpoints from file
    if (!m_divePlan->m_setPoints.loadSetPointsFromFile()) {
        // If loading failed, ensure we have at least one default setpoint
        if (m_divePlan->m_setPoints.nbOfSetPoints() == 0) {
            m_divePlan->m_setPoints.addSetPoint(0.0, 0.7);
            m_divePlan->m_setPoints.saveSetPointsToFile();
        }
    }
    
    // Make sure the setpoints are sorted
    m_divePlan->m_setPoints.sortSetPoints();
    m_divePlan->calculate();
    
    // Add a test shortcut for manual menu refresh (for debugging)
    QAction* refreshMenuAction = new QAction(this);
    refreshMenuAction->setShortcut(QKeySequence("F5"));
    // Use a different approach to handle the action
    refreshMenuAction->setProperty("activateWindow", true);
    this->addAction(refreshMenuAction);
    
    // Set up UI
    setupUI();
    
    QElapsedTimer timer;
    timer.start();
    
    // Initial refresh of tables
    refreshStopStepsTable();
    qDebug() << "Initial refreshStopStepsTable() took" << timer.elapsed() << "ms";
    
    timer.restart();
    refreshSetpointsTable();
    qDebug() << "Initial refreshSetpointsTable() took" << timer.elapsed() << "ms";
    
    timer.restart();
    refreshDivePlanTable();
    qDebug() << "Initial refreshDivePlanTable() took" << timer.elapsed() << "ms";
    
    timer.restart();
    refreshGasesTable();
    qDebug() << "Initial refreshGasesTable() took" << timer.elapsed() << "ms";
    
    // Update setpoint visibility based on current mode
    updateSetpointVisibility();
    
    // First attempt at forcing gas table column sizes
    resizeGasesTable();
    
    // Use the common window sizing and positioning function
    setWindowSizeAndPosition(this, preferredWidth, preferredHeight, WindowPosition::CENTER);
    
    // Try again after positioning
    resizeGasesTable();
    
    // Add event handler for window shown event to force column widths
    // after window is completely shown and sized
    connect(this, &QWidget::windowTitleChanged, this, &DivePlanWindow::onWindowTitleChanged);
    
    // As a last resort, use a timer that triggers after window is fully shown
    QTimer::singleShot(200, this, &DivePlanWindow::resizeGasesTable);
}

DivePlanWindow::~DivePlanWindow() {}

void DivePlanWindow::setupUI() {
    // Create central widget and main layout
    QWidget *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);
    
    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);
    
    // Initialize main splitter first
    mainSplitter = new QSplitter(Qt::Horizontal, centralWidget);
    
    // Left panel for stop steps and setpoints
    QWidget *leftPanelWidget = new QWidget(mainSplitter);
    leftPanelWidget->setMinimumWidth(COLUMN_WIDTH);
    leftPanelWidget->setMaximumWidth(COLUMN_WIDTH);
    QVBoxLayout *leftPanelLayout = new QVBoxLayout(leftPanelWidget);
    
    // Create left panel splitter for stop steps and setpoints tables
    leftPanelSplitter = new QSplitter(Qt::Vertical, leftPanelWidget);
    qDebug() << "leftPanelSplitter created:" << (leftPanelSplitter != nullptr);
    
    // Create stop steps section
    QWidget *stopStepsWidget = new QWidget(leftPanelSplitter);
    QVBoxLayout *stopStepsLayout = new QVBoxLayout(stopStepsWidget);
    
    // Stop steps header with label and add button
    QWidget *stopStepsHeader = new QWidget(stopStepsWidget);
    QHBoxLayout *stopStepsHeaderLayout = new QHBoxLayout(stopStepsHeader);
    stopStepsHeaderLayout->setContentsMargins(0, 0, 0, 0);
    
    QLabel *stopStepsLabel = new QLabel("Stop Steps:", stopStepsHeader);
    stopStepsLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    
    QPushButton *addStopStepButton = new QPushButton("+", stopStepsHeader);
    addStopStepButton->setFixedSize(24, 24);
    addStopStepButton->setToolTip("Add Stop Step");
    connect(addStopStepButton, &QPushButton::clicked, this, &DivePlanWindow::addStopStep);
    
    stopStepsHeaderLayout->addWidget(stopStepsLabel, 1);  // Give the label stretch
    stopStepsHeaderLayout->addWidget(addStopStepButton, 0);  // No stretch for button
    
    stopStepsLayout->addWidget(stopStepsHeader);
    
    // Stop steps table
    stopStepsTable = new QTableWidget(0, STOP_STEP_COLUMNS_COUNT, stopStepsWidget);
    setupStopStepsTable();
    stopStepsLayout->addWidget(stopStepsTable);
    
    // Create setpoints section
    QWidget *setpointsWidget = new QWidget(leftPanelSplitter);
    QVBoxLayout *setpointsLayout = new QVBoxLayout(setpointsWidget);
    
    // Setpoints header with label and add button
    QWidget *setpointsHeader = new QWidget(setpointsWidget);
    QHBoxLayout *setpointsHeaderLayout = new QHBoxLayout(setpointsHeader);
    setpointsHeaderLayout->setContentsMargins(0, 0, 0, 0);
    
    QLabel *setpointsLabel = new QLabel("Setpoints:", setpointsHeader);
    setpointsLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    
    QPushButton *addSetpointButton = new QPushButton("+", setpointsHeader);
    addSetpointButton->setFixedSize(24, 24);
    addSetpointButton->setToolTip("Add Setpoint");
    connect(addSetpointButton, &QPushButton::clicked, this, &DivePlanWindow::addSetpoint);
    
    setpointsHeaderLayout->addWidget(setpointsLabel, 1);  // Give the label stretch
    setpointsHeaderLayout->addWidget(addSetpointButton, 0);  // No stretch for button
    
    setpointsLayout->addWidget(setpointsHeader);
    
    // Setpoints table
    setpointsTable = new QTableWidget(0, SETPOINT_COLUMNS_COUNT, setpointsWidget);
    setupSetpointsTable();
    setpointsLayout->addWidget(setpointsTable);
    
    // Add stop steps and setpoints widget to leftPanelSplitter
    leftPanelSplitter->addWidget(stopStepsWidget);
    leftPanelSplitter->addWidget(setpointsWidget);
    
    // Add left panel splitter to left panel layout
    leftPanelLayout->addWidget(leftPanelSplitter);

    // Right panel (widgets + dive plan)
    QWidget *rightPanelWidget = new QWidget(mainSplitter);
    QVBoxLayout *rightPanelLayout = new QVBoxLayout(rightPanelWidget);
    
    // Create a splitter for the visualization area and dive plan table
    verticalSplitter = new QSplitter(Qt::Vertical, rightPanelWidget);
    qDebug() << "verticalSplitter created:" << (verticalSplitter != nullptr);
    
    // Top visualization area
    QWidget *visualizationWidget = new QWidget(verticalSplitter);
    QVBoxLayout *visualizationLayout = new QVBoxLayout(visualizationWidget);
    
    // Top widgets section - split into two equal widgets
    topWidgetsSplitter = new QSplitter(Qt::Horizontal, visualizationWidget);
    qDebug() << "topWidgetsSplitter created:" << (topWidgetsSplitter != nullptr);
    
    // First top widget (DiveStatistics)
    QWidget *topWidget1 = new QWidget(topWidgetsSplitter);
    topWidget1->setMinimumHeight(50);  // Reduced from 200
    topWidget1->setStyleSheet("background-color: #f0f0f0; border: 1px solid #ccc;");
    QVBoxLayout *topWidget1Layout = new QVBoxLayout(topWidget1);
    QLabel *diveStatsLabel = new QLabel("DiveStatistics", topWidget1);
    diveStatsLabel->setAlignment(Qt::AlignCenter);
    topWidget1Layout->addWidget(diveStatsLabel);
    
    // Second top widget (GasConsumption)
    QWidget *topWidget2 = new QWidget(topWidgetsSplitter);
    topWidget2->setMinimumHeight(50);  // Reduced from 200
    topWidget2->setStyleSheet("border: 1px solid #ccc;");
    QVBoxLayout *topWidget2Layout = new QVBoxLayout(topWidget2);
    
    // Create the gases table
    gasesTable = new QTableWidget(0, GASES_TABLE_COLUMNS_COUNT, topWidget2);
    setupGasesTable();
    
    // Add the table to the layout
    topWidget2Layout->addWidget(gasesTable);
    
    // Add both top widgets to the splitter
    topWidgetsSplitter->addWidget(topWidget1);
    topWidgetsSplitter->addWidget(topWidget2);
    
    // Add top widgets splitter to visualization layout
    visualizationLayout->addWidget(topWidgetsSplitter);
    
    // Commented out chart widget section
    // m_chartWidget = new QWidget(visualizationWidget);
    // m_chartWidget->setMinimumHeight(200);
    // m_chartWidget->setStyleSheet("border: 1px solid #ccc;");
    // visualizationLayout->addWidget(m_chartWidget, 1); // Give it stretch factor to take available space

    // Add a text label to indicate the slider functionality
    QLabel *sliderHintLabel = new QLabel("⟿ Drag to resize dive plan table ⟿", visualizationWidget);
    sliderHintLabel->setAlignment(Qt::AlignCenter);
    sliderHintLabel->setStyleSheet("color: #606060; background-color: transparent; font-size: 10px;");
    visualizationLayout->addWidget(sliderHintLabel);
    
    // Dive plan table in a separate widget
    QWidget *divePlanWidget = new QWidget(verticalSplitter);
    QVBoxLayout *divePlanLayout = new QVBoxLayout(divePlanWidget);
    divePlanLayout->setContentsMargins(0, 0, 0, 0);
    
    // Dive plan table
    divePlanTable = new QTableWidget(0, DIVE_PLAN_COLUMNS_COUNT, divePlanWidget);
    setupDivePlanTable();
    divePlanLayout->addWidget(divePlanTable);
    
    // Information panel at the bottom of the dive plan
    infoLabel = new QLabel("* Highlighted cells indicate warning conditions", divePlanWidget);
    infoLabel->setStyleSheet("color: #666;");
    divePlanLayout->addWidget(infoLabel);
    
    // Add the visualization and dive plan widgets to the vertical splitter
    verticalSplitter->addWidget(visualizationWidget);
    verticalSplitter->addWidget(divePlanWidget);
    
    // Add the vertical splitter to the right panel
    rightPanelLayout->addWidget(verticalSplitter);
    
    // Add widgets to main splitter
    mainSplitter->addWidget(leftPanelWidget);
    mainSplitter->addWidget(rightPanelWidget);
    
    // Add the main splitter to the main layout
    mainLayout->addWidget(mainSplitter);
    
    // Configure all splitters after widgets creation
    setupSplitters();
    
    // Force an immediate layout pass
    QApplication::processEvents();
}

void DivePlanWindow::rebuildDivePlan() {
    static bool isRebuilding = false;
    if (isRebuilding) {
        qDebug() << "Preventing recursive rebuildDivePlan call";
        return;
    }
    
    isRebuilding = true;
    
    QElapsedTimer timer;
    timer.start();
    
    // PERFORM THE REBUILD
    m_divePlan->build();
    qDebug() << "build() took" << timer.elapsed() << "ms";
    
    
    // Refresh the stopstep table
    timer.restart();
    refreshStopStepsTable();
    qDebug() << "refreshStopStepsTable() took" << timer.elapsed() << "ms";

    m_tableDirty = true;
    isRebuilding = false;
}

void DivePlanWindow::refreshDivePlan() {
    static bool isRefreshing = false;
    if (isRefreshing || !m_divePlan) {
        qDebug() << "Preventing recursive refreshDivePlan call or null plan";
        return;
    }
    
    isRefreshing = true;
   
    QElapsedTimer timer;
    timer.start();
    
    // Log the execution times for optimization purposes
    m_divePlan->calculate();
    qDebug() << "calculate() took" << timer.elapsed() << "ms";
    
    timer.restart();
    m_divePlan->updateGasConsumption();
    qDebug() << "updateGasConsumption() took" << timer.elapsed() << "ms";
    refreshGasesTable();
    
    timer.restart();
    // Check if the dive plan table is visible by checking its height
    bool isTableVisible = divePlanTable && divePlanTable->isVisible() && divePlanTable->height() > 0;
    
    if (isTableVisible) {
        refreshDivePlanTable();
        qDebug() << "refreshDivePlanTable() took" << timer.elapsed() << "ms";
    } else {
        // Mark the table as dirty so it will be refreshed when made visible
        m_tableDirty = true;
        qDebug() << "DivePlanTable refresh deferred (not visible)";
    }

    isRefreshing = false;
}

QString DivePlanWindow::getPhaseString(Phase phase)
{
    return QString::fromStdString(getPhaseIcon(phase));
}

QString DivePlanWindow::getStepModeString(stepMode mode)
{
    return QString::fromStdString(getStepModeIcon(mode));
}

void DivePlanWindow::highlightWarningCells() {
    
    for (int row = 0; row < divePlanTable->rowCount(); ++row) {
        // Get current values
        const DiveStep& step = m_divePlan->m_diveProfile[row];
        
        // 1. Gas Density
        QTableWidgetItem* gasItem = divePlanTable->item(row, COL_GAS_DENSITY);
        if (gasItem) {
            TableHelper::highlightCell(gasItem, step.m_gasDensity > g_parameters.m_warningGasDensity);
        }
        
        // 2. pO2 Max (too high or too low)
        QTableWidgetItem* pO2Item = divePlanTable->item(row, COL_PO2_MAX);
        if (pO2Item) {
            TableHelper::highlightCell(pO2Item, 
                (step.m_pO2Max > g_parameters.m_PpO2Deco || step.m_pO2Max < g_parameters.m_warningPpO2Low));
        }
        
        // 3. CNS Single Dive
        QTableWidgetItem* cnsItem = divePlanTable->item(row, COL_CNS_SINGLE);
        if (cnsItem) {
            TableHelper::highlightCell(cnsItem, step.m_cnsTotalSingleDive > g_parameters.m_warningCnsMax);
        }
        
        // 4. OTU Total
        QTableWidgetItem* otuItem = divePlanTable->item(row, COL_OTU);
        if (otuItem) {
            TableHelper::highlightCell(otuItem, step.m_otuTotal > g_parameters.m_warningOtuMax);
        }
    }
}

void DivePlanWindow::bailoutToggled(bool checked)
{
    // If CC mode and bailout is checked, set mode to BAILOUT
    // Otherwise, restore to original mode (CC or OC)
    if (m_divePlan->m_mode == diveMode::CC) {
        m_divePlan->m_bailout = checked;
        
        // Update menu state
        updateMenuState();
    }

    // Rebuild the dive plan
    rebuildDivePlan();
    refreshDivePlan();

    // Allow UI to process events after the edit
    QApplication::processEvents();
}

void DivePlanWindow::activate() {
    // Just tell main window to handle menu and make this window active
    if (m_mainWindow && !m_isUpdating) {
        m_isUpdating = true;
        m_mainWindow->activateWindowWithMenu(this);
        m_isUpdating = false;
    }
}

void DivePlanWindow::mouseReleaseEvent(QMouseEvent* event) {
    QMainWindow::mouseReleaseEvent(event);
    
    // When user clicks on this window, ensure it has the menu
    activate();
}

// Handles splitters
void DivePlanWindow::setupSplitters() {
    // Clear previous splitter configurations
    m_splitters.clear();
    
    // Configure all splitters
    
    // 1. Left panel splitter (stopSteps and setpoints)
    // Ensure equal initial split
    configureSplitter(
        leftPanelSplitter, 
        "leftPanelSplitter", 
        SplitterDirection::VERTICAL, 
        8, 
        "≡", 
        QList<int>() << 150 << 150, // Equal division of space
        true
    );
    
    // 2. Main splitter (left panel and right panel) - fixed width
    configureSplitter(
        mainSplitter, 
        "mainSplitter", 
        SplitterDirection::HORIZONTAL, 
        1, 
        "", 
        QList<int>() << COLUMN_WIDTH << 1080, 
        false
    );
    // Special case: Lock the main splitter
    if (mainSplitter) {
        mainSplitter->setHandleWidth(1);
        mainSplitter->setChildrenCollapsible(false);
    }
    
    // 3. Vertical splitter (visualization and dive plan)
    // Hide dive plan table by default
    configureSplitter(
        verticalSplitter, 
        "verticalSplitter", 
        SplitterDirection::VERTICAL, 
        12, 
        "≡≡≡", 
        QList<int>() << 1000 << 0, // Set dive plan to 0 height initially
        true
    );
    
    // 4. Top widgets splitter (analytics and consumption)
    configureSplitter(
        topWidgetsSplitter, 
        "topWidgetsSplitter", 
        SplitterDirection::HORIZONTAL, 
        10, 
        "⋮⋮⋮", 
        QList<int>() << 400 << 400, 
        true
    );
    
    // Process events to ensure splitters are properly laid out
    QApplication::processEvents();
    
    // Set up a timer to check splitter states periodically
    m_splitterCheckTimer = std::make_unique<QTimer>();
    m_splitterCheckTimer->setParent(this); // Still set Qt parent for event system
    connect(m_splitterCheckTimer.get(), &QTimer::timeout, [this]() {
        for (const auto& config : m_splitters) {
            if (config.splitter) {
                updateSplitterVisibility(config.splitter);
                
                // Special case handling for the gases table
                if (config.name == "topWidgetsSplitter") {
                    QList<int> sizes = config.splitter->sizes();
                    if (sizes[1] > 0) {
                        resizeGasesTable();
                    }
                }
            }
        }
    });
    m_splitterCheckTimer->start(500); // Check every half second
}

void DivePlanWindow::configureSplitter(
    QSplitter* splitter, 
    const QString& name, 
    SplitterDirection direction, 
    int handleWidth, 
    const QString& handleDecoration, 
    const QList<int>& defaultSizes, 
    bool collapsible) 
{
    if (!splitter) return;
    
    // Set splitter name for identification
    splitter->setObjectName(name);
    
    // Configure basic properties - CRITICAL: Allow children to collapse to make handles work properly
    splitter->setHandleWidth(handleWidth);
    splitter->setChildrenCollapsible(true); // Allow widgets to collapse to zero
    splitter->setOpaqueResize(true);
    
    // Apply enhanced handle style for better visibility
    QString handleStyle;
    if (direction == SplitterDirection::VERTICAL) {
        handleStyle = "QSplitter::handle {"
            "   background-color: #4a90e2;"  // More visible blue color
            "   border-top: 1px solid #3a80d2;"
            "   border-bottom: 1px solid #3a80d2;"
            "   min-height: " + QString::number(handleWidth) + "px;"
            "   height: " + QString::number(handleWidth) + "px;"
            "   max-height: " + QString::number(handleWidth) + "px;"
            "}"
            "QSplitter::handle:hover {"
            "   background-color: #2a70c2;"
            "   border-top: 1px solid #1a60b2;"
            "   border-bottom: 1px solid #1a60b2;"
            "}";
    } else {
        handleStyle = "QSplitter::handle {"
            "   background-color: #4a90e2;"  // More visible blue color
            "   border-left: 1px solid #3a80d2;"
            "   border-right: 1px solid #3a80d2;"
            "   min-width: " + QString::number(handleWidth) + "px;"
            "   width: " + QString::number(handleWidth) + "px;"
            "   max-width: " + QString::number(handleWidth) + "px;"
            "}"
            "QSplitter::handle:hover {"
            "   background-color: #2a70c2;"
            "   border-left: 1px solid #1a60b2;"
            "   border-right: 1px solid #1a60b2;"
            "}";
    }
    splitter->setStyleSheet(handleStyle);
    
    // Set initial sizes
    if (!defaultSizes.isEmpty()) {
        splitter->setSizes(defaultSizes);
    }
    
    // Add handle decoration if specified
    if (!handleDecoration.isEmpty() && splitter->count() > 1) {
        for (int i = 1; i < splitter->count(); i++) {
            QSplitterHandle* handle = splitter->handle(i);
            if (handle) {
                // Clear any existing layout
                if (handle->layout()) {
                    QLayoutItem* item;
                    while ((item = handle->layout()->takeAt(0)) != nullptr) {
                        delete item->widget();
                        delete item;
                    }
                    delete handle->layout();
                }
                
                // Create new layout with decoration
                QBoxLayout* layout = (direction == SplitterDirection::VERTICAL) 
                    ? static_cast<QBoxLayout*>(new QHBoxLayout(handle))
                    : static_cast<QBoxLayout*>(new QVBoxLayout(handle));
                    
                layout->setContentsMargins(0, 0, 0, 0);
                QLabel* decorationLabel = new QLabel(handleDecoration, handle);
                decorationLabel->setStyleSheet("color: white; background: transparent;");
                decorationLabel->setAlignment(Qt::AlignCenter);
                layout->addWidget(decorationLabel);
                
                // Add tooltip
                handle->setToolTip("Drag to resize panels");
            }
        }
    }
    
    // Connect the splitter movement signal
    connect(splitter, &QSplitter::splitterMoved, this, &DivePlanWindow::onSplitterMoved);
    
    // Add to tracked splitters
    SplitterConfig config;
    config.splitter = splitter;
    config.name = name;
    config.direction = direction;
    config.handleWidth = handleWidth;
    config.handleDecoration = handleDecoration;
    config.defaultSizes = defaultSizes;
    config.collapsible = collapsible;
    
    m_splitters.append(config);
}

void DivePlanWindow::handleSplitterMovement(QSplitter* splitter, int /*index*/) {
    if (!splitter) return;
    
    // Find the config for this splitter
    SplitterConfig* config = nullptr;
    for (auto& cfg : m_splitters) {
        if (cfg.splitter == splitter) {
            config = &cfg;
            break;
        }
    }
    
    if (!config) return;
    
    QList<int> sizes = splitter->sizes();
    int totalSize = 0;
    for (int size : sizes) {
        totalSize += size;
    }
    
    // Handle special case for the main splitter (fixed width)
    if (config->name == "mainSplitter") {
        // Ensure left panel stays at fixed width
        if (sizes[0] != COLUMN_WIDTH) {
            splitter->blockSignals(true);
            splitter->setSizes(QList<int>() << COLUMN_WIDTH << (totalSize - COLUMN_WIDTH));
            splitter->blockSignals(false);
        }
    }
    // Handle all other splitters that allow collapsing
    else if (config->collapsible) {
        bool needsResize = false;
        
        // Check if any section needs to be collapsed
        for (int i = 0; i < sizes.size(); i++) {
            // If widget is being dragged to a small size, collapse it completely
            if (sizes[i] > 0 && sizes[i] < config->snapThreshold) {
                sizes[i] = 0; // Set to zero for true collapse
                
                // Redistribute remaining space
                int remainingSpaceIndex = -1;
                for (int j = 0; j < sizes.size(); j++) {
                    if (j != i && sizes[j] > 0) {
                        remainingSpaceIndex = j;
                        break;
                    }
                }
                
                if (remainingSpaceIndex >= 0) {
                    sizes[remainingSpaceIndex] = totalSize;
                }
                
                needsResize = true;
            }
        }
        
        // Special case for topWidgetsSplitter - prevent both sides from collapsing
        if (config->name == "topWidgetsSplitter") {
            if (sizes[0] <= 0 && sizes[1] <= 0) {
                sizes[0] = totalSize / 2;
                sizes[1] = totalSize / 2;
                needsResize = true;
            }
        }
        
        // Apply size changes if needed
        if (needsResize) {
            splitter->blockSignals(true);
            splitter->setSizes(sizes);
            splitter->blockSignals(false);
        }
    }
    
    // Update visibility of internal widgets based on new sizes
    for (int i = 0; i < splitter->count() && i < sizes.size(); i++) {
        QWidget* widget = splitter->widget(i);
        if (!widget) continue;
        
        bool isCollapsed = (sizes[i] <= 0);
        
        // Handle specific widget content based on splitter type
        if (config->name == "leftPanelSplitter") {
            if (i == 0 && stopStepsTable) {
                stopStepsTable->setVisible(!isCollapsed);
            } 
            else if (i == 1 && setpointsTable) {
                bool isClosedCircuit = (m_divePlan && m_divePlan->m_mode == diveMode::CC);
                setpointsTable->setVisible(!isCollapsed && isClosedCircuit);
            }
        }
        else if (config->name == "verticalSplitter") {
            if (i == 1) { // Dive plan section
                bool wasHidden = false;
                if (divePlanTable) {
                    wasHidden = !divePlanTable->isVisible();
                    divePlanTable->setVisible(!isCollapsed);
                }
                if (infoLabel) {
                    infoLabel->setVisible(!isCollapsed);
                }
                
                // Handle refresh logic for dive plan
                if (!isCollapsed) {
                    if (m_tableDirty || wasHidden) {
                        // Refresh immediately if becoming visible
                        if (wasHidden) {
                            refreshDivePlanTable();
                            QTimer::singleShot(50, this, &DivePlanWindow::resizeDivePlanTable);
                        } else {
                            QTimer::singleShot(0, this, &DivePlanWindow::refreshDivePlanTable);
                        }
                        m_tableDirty = false;
                    }
                    
                    // Always trigger a resize when making visible
                    QTimer::singleShot(10, this, &DivePlanWindow::resizeDivePlanTable);
                    
                    // Also try to resize immediately
                    resizeDivePlanTable();
                }
                if (isCollapsed) {
                    m_tableDirty = true;
                }
            }
        }
    }
    
    // Make sure all handles are visible
    for (int i = 1; i < splitter->count(); ++i) {
        QSplitterHandle* handle = splitter->handle(i);
        if (handle) {
            handle->setVisible(true);
        }
    }
    
    // Special case for setpoints visibility
    if (config->name == "leftPanelSplitter") {
        updateSetpointVisibility();
    }
    
    // Special case for gases table
    if (config->name == "topWidgetsSplitter") {
        if (sizes[1] > 0) {
            resizeGasesTable();
            QTimer::singleShot(10, this, &DivePlanWindow::resizeGasesTable);
        }
    }
}

void DivePlanWindow::updateWidgetVisibility(QSplitter* splitter, const QList<int>& sizes) {
    if (!splitter) return;
    
    // Determine splitter type by name
    QString splitterName = splitter->objectName();
    
    // Process each widget in the splitter
    for (int i = 0; i < splitter->count() && i < sizes.size(); i++) {
        QWidget* widget = splitter->widget(i);
        if (!widget) continue;
        
        // Always keep the container widget visible to maintain handle visibility
        widget->setVisible(true);
        
        // Whether this section is effectively hidden (size = 0)
        bool isEffectivelyHidden = (sizes[i] == 0);
        
        // Handle specific widgets based on splitter type
        if (splitterName == "leftPanelSplitter") {
            if (i == 0) { // Stop steps section
                if (stopStepsTable) {
                    stopStepsTable->setVisible(!isEffectivelyHidden);
                }
            } 
            else if (i == 1) { // Setpoints section
                if (setpointsTable) {
                    bool isClosedCircuit = (m_divePlan && m_divePlan->m_mode == diveMode::CC);
                    setpointsTable->setVisible(!isEffectivelyHidden && isClosedCircuit);
                }
            }
        }
        else if (splitterName == "verticalSplitter") {
            if (i == 1) { // Dive plan section
                bool showDivePlan = !isEffectivelyHidden;
                
                // Update table visibility
                if (divePlanTable) {
                    divePlanTable->setVisible(showDivePlan);
                }
                if (infoLabel) {
                    infoLabel->setVisible(showDivePlan);
                }
                
                // Handle refresh logic for previously hidden table
                if (showDivePlan && m_tableDirty) {
                    QTimer::singleShot(0, this, &DivePlanWindow::refreshDivePlanTable);
                    m_tableDirty = false;
                }
                if (!showDivePlan) {
                    m_tableDirty = true;
                }
            }
        }
        else if (splitterName == "topWidgetsSplitter") {
            // Both sections of top splitter are handled generically
            // Just ensure we resize the gases table when its section is visible
            if (i == 1 && !isEffectivelyHidden) { // Gas consumption section
                QTimer::singleShot(0, this, &DivePlanWindow::resizeGasesTable);
            }
        }
    }
    
    // Special case for setpoints visibility
    if (splitterName == "leftPanelSplitter") {
        updateSetpointVisibility();
    }
}

void DivePlanWindow::updateSplitterVisibility(QSplitter* splitter) {
    if (!splitter) return;
    
    QList<int> sizes = splitter->sizes();
    
    // Update visibility for each widget based on size
    for (int i = 0; i < splitter->count() && i < sizes.size(); i++) {
        QWidget* widget = splitter->widget(i);
        if (widget) {
            // Always keep widget visible for handle visibility
            widget->setVisible(true);
            
            // Determine if the widget is effectively "collapsed"
            bool isEffectivelyHidden = (sizes[i] == 0);
            
            // Special handling for specific widgets based on splitter type
            if (splitter->objectName() == "leftPanelSplitter") {
                if (i == 0 && stopStepsTable) {
                    stopStepsTable->setVisible(!isEffectivelyHidden);
                }
                else if (i == 1) {
                    bool isClosedCircuit = (m_divePlan && m_divePlan->m_mode == diveMode::CC);
                    // Hide setpoints table if not in CC mode or if panel is effectively hidden
                    if (setpointsTable) {
                        setpointsTable->setVisible(isClosedCircuit && !isEffectivelyHidden);
                    }
                }
            }
            else if (splitter->objectName() == "verticalSplitter") {
                if (i == 1) { // Dive plan section
                    if (divePlanTable) {
                        divePlanTable->setVisible(!isEffectivelyHidden);
                        // Mark table dirty if hidden
                        if (isEffectivelyHidden) {
                            m_tableDirty = true;
                        }
                    }
                    if (infoLabel) {
                        infoLabel->setVisible(!isEffectivelyHidden);
                    }
                }
            }
        }
    }
    
    // Ensure all handles are visible even at extreme positions
    for (int i = 1; i < splitter->count(); ++i) {
        QSplitterHandle* handle = splitter->handle(i);
        if (handle) {
            handle->setVisible(true);
        }
    }
}// Update the visibility of setpoints based on the current dive mode

void DivePlanWindow::updateSetpointVisibility() {
    if (!m_divePlan) return;
    
    bool isClosedCircuit = (m_divePlan->m_mode == diveMode::CC);
    
    // Find the left panel splitter config
    for (const auto& config : m_splitters) {
        if (config.name == "leftPanelSplitter" && config.splitter) {
            QList<int> sizes = config.splitter->sizes();
            
            // If setpoints panel should be visible (CC mode)
            if (isClosedCircuit) {
                // If setpoints is collapsed but should be visible
                if (sizes[1] == 0) {
                    // Show setpoints panel with a reasonable size
                    if (sizes[0] > 0) {
                        // If stop steps is visible, split space evenly
                        int totalSize = sizes[0];
                        sizes[0] = totalSize / 2;
                        sizes[1] = totalSize / 2;
                    } else {
                        // If both are collapsed, give each a default size
                        sizes[0] = 100;
                        sizes[1] = 100;
                    }
                    config.splitter->setSizes(sizes);
                }
                
                // Ensure setpoints table is visible
                if (setpointsTable) {
                    setpointsTable->setVisible(true);
                }
            } 
            // Not CC mode, completely hide setpoints
            else {
                if (sizes[1] > 0) {
                    // Completely hide setpoints panel by setting to 0
                    sizes[1] = 0;
                    config.splitter->setSizes(sizes);
                }
                
                // Hide setpoints table
                if (setpointsTable) {
                    setpointsTable->setVisible(false);
                }
            }
            break;
        }
    }
    
    // Process events to ensure UI updates
    QApplication::processEvents();
}

void DivePlanWindow::showEvent(QShowEvent* event) {
    QMainWindow::showEvent(event);
    
    // Activate this window when shown
    activate();
    
    // Set up splitters
    QTimer::singleShot(50, this, &DivePlanWindow::initializeSplitters);
    
    // Set up a sequence of resize attempts to handle race conditions
    QTimer::singleShot(100, this, &DivePlanWindow::resizeGasesTable);
    QTimer::singleShot(200, this, &DivePlanWindow::resizeDivePlanTable);
    QTimer::singleShot(500, this, &DivePlanWindow::resizeDivePlanTable);
    
    // Additional fallback for when layout gets properly settled
    QTimer::singleShot(800, this, [this]() {
        if (divePlanTable && divePlanTable->isVisible()) {
            resizeDivePlanTable();
        }
        resizeGasesTable();
    });
}

void DivePlanWindow::resizeEvent(QResizeEvent* event) {
    QMainWindow::resizeEvent(event);
    
    // Only resize the dive plan table if it's visible
    if (divePlanTable && divePlanTable->isVisible() && divePlanTable->height() > 0) {
        resizeDivePlanTable();
    }
    
    // Always resize the gases table
    if (gasesTable) {
        resizeGasesTable();
    }
}

// Placeholder action implementations
void DivePlanWindow::defineMission()
{
    // Placeholder for Define mission functionality
    qDebug() << "Define mission action triggered";
}

void DivePlanWindow::setMaxTime(){
    std::pair<double, double> result = m_divePlan->getMaxTimeAndTTS();
    std::cout << "Max Time: " << result.first << " Max TTS: " << result.second << std::endl;

    // find the first stop step
    int firstStopIndex = 0;
    for (int i = 1; i < m_divePlan->nbOfSteps(); i++) {
        if (m_divePlan->m_diveProfile[i].m_phase == Phase::STOP) {
            firstStopIndex = i;
            break;
        }
    }
    m_divePlan->m_diveProfile[firstStopIndex].m_time = result.first;
    m_divePlan->calculate();
    refreshDivePlan();
    refreshStopStepsTable();
}

void DivePlanWindow::optimiseDecoGas()
{
    // Placeholder for Optimise a deco gas functionality
    qDebug() << "Optimise a deco gas action triggered";
    m_divePlan->printSummary();
    m_divePlan->optimiseDecoGas();
}

void DivePlanWindow::onWindowTitleChanged()
{
    static bool firstActivation = true;
    if (firstActivation) {
        firstActivation = false;
        QTimer::singleShot(100, this, &DivePlanWindow::resizeGasesTable);
    }
}

void DivePlanWindow::initializeSplitters()
{
    // Initialize all splitters
    for (const auto& config : m_splitters) {
        if (config.splitter) {
            // Special case for vertical splitter: ensure dive plan is hidden initially
            if (config.name == "verticalSplitter") {
                config.splitter->setSizes(QList<int>() << config.splitter->height() << 0);
                
                // Hide the dive plan content
                if (divePlanTable) divePlanTable->setVisible(false);
                if (infoLabel) infoLabel->setVisible(false);
            }
            
            // Special case for left panel splitter: ensure equal starting split when in CC mode
            else if (config.name == "leftPanelSplitter" && m_divePlan) {
                bool isClosedCircuit = (m_divePlan->m_mode == diveMode::CC);
                if (isClosedCircuit) {
                    config.splitter->setSizes(QList<int>() << config.splitter->height()/2 << config.splitter->height()/2);
                } else {
                    // Hide setpoints completely in OC mode
                    config.splitter->setSizes(QList<int>() << config.splitter->height() << 0);
                    if (setpointsTable) setpointsTable->setVisible(false);
                }
            }
            
            // Ensure all handles are visible after setting sizes
            for (int i = 1; i < config.splitter->count(); ++i) {
                QSplitterHandle* handle = config.splitter->handle(i);
                if (handle) {
                    handle->setVisible(true);
                }
            }
        }
    }
    
    // Force table resizing
    QTimer::singleShot(10, this, &DivePlanWindow::resizeDivePlanTable);
    QTimer::singleShot(10, this, &DivePlanWindow::resizeGasesTable);
    
    // Also attempt immediate resizing
    if (divePlanTable && divePlanTable->isVisible() && divePlanTable->height() > 0) {
        resizeDivePlanTable();
    }
    resizeGasesTable();
}

// Add this implementation to dive_plan_gui.cpp:
void DivePlanWindow::onSplitterMoved(int /*pos*/, int index) {
    QSplitter* splitter = qobject_cast<QSplitter*>(sender());
    if (splitter) {
        handleSplitterMovement(splitter, index);
    }
}

void DivePlanWindow::ccModeActivated() {
    QElapsedTimer timer;
    timer.start();
    
    // Set CC mode
    m_divePlan->m_mode = diveMode::CC;
    
    // Update the mode display
    updateMenuState();
    
    // Show setpoints table if in CC mode
    updateSetpointVisibility();
    
    // Rebuild and refresh the dive plan
    rebuildDivePlan();
    refreshDivePlan();
    
    qDebug() << "CC Mode switch took" << timer.elapsed() << "ms";
    
    // Allow UI to process events after the edit
    QApplication::processEvents();
}

void DivePlanWindow::showProgressDialog(const QString& message) {
    if (!m_progressDialog) {
        m_progressDialog = std::make_unique<QProgressDialog>(message, "Cancel", 0, 0, this);
        m_progressDialog->setWindowModality(Qt::WindowModal);
        m_progressDialog->setCancelButton(nullptr); // Optional if no cancel needed
    } else {
        m_progressDialog->setLabelText(message);
    }
    m_progressDialog->show();
}


} // namespace DiveComputer