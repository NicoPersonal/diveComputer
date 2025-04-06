#include "dive_plan_gui.hpp"
#include "main_gui.hpp"

namespace DiveComputer {

void DivePlanWindow::setpointCellChanged(int row, int column) {
    // Only handle valid columns (depth and setpoint)
    if (column == SP_COL_DEPTH || column == SP_COL_SETPOINT) {
        // Get the current value from the table
        QTableWidgetItem* item = setpointsTable->item(row, column);
        if (!item) return;
        
        bool ok;
        double value = item->text().toDouble(&ok);
        
        if (ok) {
            QElapsedTimer timer;
            timer.start();
            
            // Validate the row index is within bounds
            if (row < 0 || row >= static_cast<int>(m_divePlan->m_setPoints.nbOfSetPoints())) {
                return;
            }
            
            // Get current values
            double depth = m_divePlan->m_setPoints.m_depths[row];
            double setpoint = m_divePlan->m_setPoints.m_setPoints[row];
            
            // Update with new value
            if (column == SP_COL_DEPTH) {
                depth = value;
            } else { // SP_COL_SETPOINT
                setpoint = value;
            }
            
            // Update the setpoint
            m_divePlan->m_setPoints.m_depths[row] = depth;
            m_divePlan->m_setPoints.m_setPoints[row] = setpoint;
            m_divePlan->m_setPoints.sortSetPoints();
            
            // Save setpoints to file
            m_divePlan->m_setPoints.saveSetPointsToFile();
            
            qDebug() << "editSetPoint() took" << timer.elapsed() << "ms";

            // Unlike stop steps, we don't need to rebuild for setpoint changes
            // We just need to refresh the dive plan to recalculate with new setpoints
            refreshDivePlan();
            
            // Since sorting might have changed positions, refresh the table
            refreshSetpointsTable();

            // Allow UI to process events after the edit
            QApplication::processEvents();
        }
    }
}

void DivePlanWindow::addSetpoint() {
    static bool isUpdating = false;
    if (isUpdating) return;
    isUpdating = true;
    
    QElapsedTimer timer;
    timer.start();
    
    // Get the last setpoint as a reference
    double lastDepth = 0.0;
    double lastSetpoint = 0.7; // Default setpoint
    
    if (m_divePlan->m_setPoints.nbOfSetPoints() > 0) {
        lastDepth = m_divePlan->m_setPoints.m_depths[0];
        lastSetpoint = m_divePlan->m_setPoints.m_setPoints[0];
    }
    
    // Add a new setpoint with similar values
    m_divePlan->m_setPoints.addSetPoint(lastDepth, lastSetpoint);
    
    // Save setpoints to file
    m_divePlan->m_setPoints.saveSetPointsToFile();
    
    qDebug() << "addSetPoint() took" << timer.elapsed() << "ms";
    
    timer.restart();
    // Refresh the setpoint table
    refreshSetpointsTable();
    qDebug() << "refreshSetpointsTable() took" << timer.elapsed() << "ms";

    // Refresh the dive plan WITHOUT rebuilding
    refreshDivePlan();

    // Allow UI to process events after the edit
    QApplication::processEvents();
    
    isUpdating = false;
}

void DivePlanWindow::deleteSetpoint(int row) {
    // Validate row index is within bounds
    if (row < 0 || row >= static_cast<int>(m_divePlan->m_setPoints.nbOfSetPoints())) {
        return;
    }
    
    // Ensure we maintain at least one setpoint
    if (m_divePlan->m_setPoints.nbOfSetPoints() > 1) {
        QElapsedTimer timer;
        timer.start();
        
        // Remove the specified setpoint
        m_divePlan->m_setPoints.removeSetPoint(row);
        
        // Save setpoints to file
        m_divePlan->m_setPoints.saveSetPointsToFile();
        
        qDebug() << "removeSetPoint() took" << timer.elapsed() << "ms";
        
        timer.restart();
        // Refresh the setpoint table
        refreshSetpointsTable();
        qDebug() << "refreshSetpointsTable() took" << timer.elapsed() << "ms";

        // Refresh the dive plan
        refreshDivePlan();

        // Allow UI to process events after the edit
        QApplication::processEvents();
    }
}

void DivePlanWindow::setupSetpointsTable() {
    // Set up column headers
    QStringList headers;
    headers << "Depth\n(m)" << "Setpoint\n(bar)" << "";
    
    // Configure table
    TableHelper::configureTable(setpointsTable, QAbstractItemView::SelectItems);
    TableHelper::setHeaders(setpointsTable, headers);
    
    // Set column widths
    setpointsTable->setColumnWidth(SP_COL_DEPTH, 60);
    setpointsTable->setColumnWidth(SP_COL_SETPOINT, 60);
    setpointsTable->setColumnWidth(SP_COL_DELETE, 45);
    
    // Connect cell change signal
    connect(setpointsTable, &QTableWidget::cellChanged, this, &DivePlanWindow::setpointCellChanged);
}

void DivePlanWindow::refreshSetpointsTable() {
    // Use the TableHelper for safe update
    TableHelper::safeUpdate(setpointsTable, this, &DivePlanWindow::setpointCellChanged, [this]() {
        // Set number of rows
        setpointsTable->setRowCount(m_divePlan->m_setPoints.nbOfSetPoints());
        
        // Add setpoints to table
        for (int i = 0; i < (int) m_divePlan->m_setPoints.nbOfSetPoints(); ++i) {
            // Depth item
            setpointsTable->setItem(i, SP_COL_DEPTH, 
                TableHelper::createNumericCell(m_divePlan->m_setPoints.m_depths[i], 1, true));
            
            // Setpoint item
            setpointsTable->setItem(i, SP_COL_SETPOINT, 
                TableHelper::createNumericCell(m_divePlan->m_setPoints.m_setPoints[i], 2, true));
            
            // Delete button (except for the last row if there's only one)
            if (m_divePlan->m_setPoints.nbOfSetPoints() > 1 || i < (int) m_divePlan->m_setPoints.nbOfSetPoints() - 1) {
                setpointsTable->setCellWidget(i, SP_COL_DELETE, createDeleteButtonWidget([this, i]() {
                    deleteSetpoint(i);
                }).release());
            }
        }
    });
}

}// namespace DiveComputer