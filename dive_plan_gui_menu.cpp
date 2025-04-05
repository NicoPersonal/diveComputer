#include "dive_plan_gui.hpp"
#include "main_gui.hpp"

namespace DiveComputer {

void DivePlanWindow::setupMenu() {
    if (!m_divePlanningMenu) {
        return;
    }
    
    // Clear any existing actions
    m_divePlanningMenu->clear();
    
    // Create CC Mode action
    m_ccModeAction = new QAction("CC Mode", this);
    m_ccModeAction->setCheckable(true);
    m_ccModeAction->setChecked(m_divePlan->m_mode == diveMode::CC);

    connect(m_ccModeAction, &QAction::triggered, this, &DivePlanWindow::ccModeActivated);

    m_divePlanningMenu->addAction(m_ccModeAction);
    
    // Create Bailout action (only visible in CC mode)
    m_bailoutAction = new QAction("Bailout", this);
    m_bailoutAction->setCheckable(true);
    m_bailoutAction->setChecked(m_divePlan->m_bailout);
    m_bailoutAction->setVisible(m_divePlan->m_mode == diveMode::CC);
    connect(m_bailoutAction, &QAction::triggered, [this]() {
        QElapsedTimer timer;
        timer.start();
        
        // Toggle bailout mode
        m_divePlan->m_bailout = m_bailoutAction->isChecked();
        
        // Update the display
        rebuildDivePlan();
        refreshDivePlan();
        
        qDebug() << "Bailout toggle took" << timer.elapsed() << "ms";
        
        // Allow UI to process events after the edit
        QApplication::processEvents();
    });
    m_divePlanningMenu->addAction(m_bailoutAction);
    
    // Create SP Boosted action (only visible in CC mode)
    m_gfBoostedAction = new QAction("SP Boosted", this);
    m_gfBoostedAction->setCheckable(true);
    m_gfBoostedAction->setChecked(m_divePlan->m_boosted);
    m_gfBoostedAction->setVisible(m_divePlan->m_mode == diveMode::CC);
    connect(m_gfBoostedAction, &QAction::triggered, [this]() {
        QElapsedTimer timer;
        timer.start();
        
        // Toggle SP Boosted mode
        m_divePlan->m_boosted = m_gfBoostedAction->isChecked();
        
        // Update the display - only needs calculate() which is called by refreshDivePlan()
        refreshDivePlan();
        
        qDebug() << "SP Boosted toggle took" << timer.elapsed() << "ms";
        
        // Allow UI to process events after the edit
        QApplication::processEvents();
    });
    m_divePlanningMenu->addAction(m_gfBoostedAction);
    
    // Create OC Mode action
    m_ocModeAction = new QAction("OC Mode", this);
    m_ocModeAction->setCheckable(true);
    m_ocModeAction->setChecked(m_divePlan->m_mode == diveMode::OC);
    connect(m_ocModeAction, &QAction::triggered, [this]() {
        QElapsedTimer timer;
        timer.start();
        
        // Set OC mode
        m_divePlan->m_mode = diveMode::OC;
        
        // Update the mode display
        updateMenuState();
        
        // Hide setpoints table if in OC mode
        updateSetpointVisibility();
        
        // Rebuild and refresh the dive plan
        rebuildDivePlan();
        refreshDivePlan();
        
        qDebug() << "OC Mode switch took" << timer.elapsed() << "ms";
        
        // Allow UI to process events after the edit
        QApplication::processEvents();
    });
    m_divePlanningMenu->addAction(m_ocModeAction);
    
    // Add separator
    m_divePlanningMenu->addSeparator();
    
    // Define mission action
    QAction* defineMissionAction = new QAction("Define mission", this);
    connect(defineMissionAction, &QAction::triggered, this, &DivePlanWindow::defineMission);
    m_divePlanningMenu->addAction(defineMissionAction);
    
    // Max time action
    QAction* maxTimeAction = new QAction("Max time", this);
    connect(maxTimeAction, &QAction::triggered, this, &DivePlanWindow::setMaxTime);
    m_divePlanningMenu->addAction(maxTimeAction);
    
    // Optimise a deco gas action
    QAction* optimiseDecoGasAction = new QAction("Optimise a deco gas", this);
    connect(optimiseDecoGasAction, &QAction::triggered, this, &DivePlanWindow::optimiseDecoGas);
    m_divePlanningMenu->addAction(optimiseDecoGasAction);
}

void DivePlanWindow::updateMenuState() {
    if (!m_divePlanningMenu || !m_divePlan) return;
    
    // Update the checked state of the mode actions
    m_ccModeAction->setChecked(m_divePlan->m_mode == diveMode::CC);
    m_ocModeAction->setChecked(m_divePlan->m_mode == diveMode::OC);
    
    // Update visibility and checked state of CC-specific actions
    bool inCCMode = (m_divePlan->m_mode == diveMode::CC);
    
    // Update visibility
    m_bailoutAction->setVisible(inCCMode);
    m_gfBoostedAction->setVisible(inCCMode);
    
    // Update checked states
    if (inCCMode) {
        m_bailoutAction->setChecked(m_divePlan->m_bailout);
        m_gfBoostedAction->setChecked(m_divePlan->m_boosted);
    } else {
        m_bailoutAction->setChecked(false);
        m_gfBoostedAction->setChecked(false);
    }
}

void DivePlanWindow::diveModeChanged(int index) {
    QElapsedTimer timer;
    timer.start();
    
    // Update dive plan mode
    diveMode newMode = static_cast<diveMode>(index);
    m_divePlan->m_mode = newMode;
    
    // Update menu state
    updateMenuState();
    
    // Update setpoint table visibility
    updateSetpointVisibility();

    // Rebuild the dive plan
    rebuildDivePlan();
    refreshDivePlan();
    
    qDebug() << "diveModeChanged() took" << timer.elapsed() << "ms";

    // Allow UI to process events after the edit
    QApplication::processEvents();
}

void DivePlanWindow::setDivePlanningMenu(QMenu* menu) {
    // Store menu reference
    m_divePlanningMenu = menu;
    
    // Setup menu content if we have a valid menu
    if (m_divePlanningMenu) {
        // Clear existing actions
        m_divePlanningMenu->clear();
        
        // Set up the menu with action items
        setupMenu();
        updateMenuState();
    }
}

} // namespace DiveComputer