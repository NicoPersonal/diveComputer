#include "dive_plan_gui.hpp"
#include "main_gui.hpp"

namespace DiveComputer {

void DivePlanWindow::stopStepCellChanged(int row, int column){
    // Only handle valid columns (depth and time)
    if (column == STOP_COL_DEPTH || column == STOP_COL_TIME) {
        // Get the current value from the table
        QTableWidgetItem* item = stopStepsTable->item(row, column);
        if (!item) return;
        
        bool ok;
        double value = item->text().toDouble(&ok);
        
        if (ok) {
            // Get current values
            double depth = m_divePlan->m_stopSteps.m_stopSteps[row].m_depth;
            double time = m_divePlan->m_stopSteps.m_stopSteps[row].m_time;
            
            // Update with new value
            if (column == STOP_COL_DEPTH) {
                depth = value;
            } else { // STOP_COL_TIME
                time = value;
            }
            
            // Update the stop step
            m_divePlan->m_stopSteps.editStopStep(row, depth, time);

            // Rebuild and refreshthe dive plan
            rebuildDivePlan();
            refreshDivePlan();

            // Allow UI to process events after the edit
            QApplication::processEvents();
        }
    }
}

void DivePlanWindow::addStopStep() {
    if (m_isUpdating) {
        qDebug() << "Skipping addStopStep() - already updating";
        return;
    }
    
    qDebug() << "Adding stop step - START";
    m_isUpdating = true;

    // Get the deepest stop step as a reference
    double lastDepth = 0.0;
    double lastTime = 0.0;
    
    if (m_divePlan->m_stopSteps.nbOfStopSteps() > 0) {
        lastDepth = m_divePlan->m_stopSteps.m_stopSteps[0].m_depth;
        lastTime = m_divePlan->m_stopSteps.m_stopSteps[0].m_time;
    }
    
    qDebug() << "Before adding - Stop steps count:" << m_divePlan->m_stopSteps.nbOfStopSteps();
    
    // Add a new stop step with similar values
    m_divePlan->m_stopSteps.addStopStep(lastDepth, lastTime);
    
    qDebug() << "After adding - Stop steps count:" << m_divePlan->m_stopSteps.nbOfStopSteps();
    
    // Force immediate update of the stop steps table
    stopStepsTable->setUpdatesEnabled(false);
    refreshStopStepsTable();
    stopStepsTable->setUpdatesEnabled(true);
    stopStepsTable->repaint();
    
    QApplication::processEvents();
    
    // Rebuild the dive plan separately
    rebuildDivePlan();
    refreshDivePlan();

    qDebug() << "Adding stop step - END";
    m_isUpdating = false;
    
    // As a fallback, schedule another refresh after a short delay
    QTimer::singleShot(100, this, [this]() {
        if (!m_isUpdating) {
            refreshStopStepsTable();
            stopStepsTable->repaint();
        }
    });
}

void DivePlanWindow::deleteStopStep(int row) {
    if (m_isUpdating) {
        qDebug() << "Skipping deleteStopStep() - already updating";
        return;
    }
    
    qDebug() << "Deleting stop step" << row << "- START";
    m_isUpdating = true;

    // Ensure we maintain at least one stop step
    if (m_divePlan->m_stopSteps.nbOfStopSteps() > 1) {
        qDebug() << "Before deleting - Stop steps count:" << m_divePlan->m_stopSteps.nbOfStopSteps();
        
        // Remove the specified stop step
        m_divePlan->m_stopSteps.removeStopStep(row);
        
        qDebug() << "After deleting - Stop steps count:" << m_divePlan->m_stopSteps.nbOfStopSteps();
        
        // Force immediate update of the stop steps table
        stopStepsTable->setUpdatesEnabled(false);
        refreshStopStepsTable();
        stopStepsTable->setUpdatesEnabled(true);
        stopStepsTable->repaint();
        
        QApplication::processEvents();
        
        // Rebuild the dive plan separately
        rebuildDivePlan();
        refreshDivePlan();
    }

    qDebug() << "Deleting stop step - END";
    m_isUpdating = false;
    
    // As a fallback, schedule another refresh after a short delay
    QTimer::singleShot(100, this, [this]() {
        if (!m_isUpdating) {
            refreshStopStepsTable();
            stopStepsTable->repaint();
        }
    });
}

void DivePlanWindow::setupStopStepsTable()
{
    // Set up column headers
    QStringList headers;
    headers << "Depth\n(m)" << "Time\n(min)" << "";
    stopStepsTable->setHorizontalHeaderLabels(headers);
    
    // Configure table properties
    stopStepsTable->setSelectionBehavior(QAbstractItemView::SelectItems);
    stopStepsTable->setSelectionMode(QAbstractItemView::SingleSelection);
    stopStepsTable->setAlternatingRowColors(true);
    stopStepsTable->verticalHeader()->setVisible(false);
    
    // Set column widths
    stopStepsTable->setColumnWidth(STOP_COL_DEPTH, 60);
    stopStepsTable->setColumnWidth(STOP_COL_TIME,  60);
    stopStepsTable->setColumnWidth(STOP_COL_DELETE, 45);
    
    // Set edit triggers
    stopStepsTable->setEditTriggers(QAbstractItemView::DoubleClicked | 
                                   QAbstractItemView::SelectedClicked | 
                                   QAbstractItemView::EditKeyPressed);
    
    // Connect cell change signal
    connect(stopStepsTable, &QTableWidget::cellChanged, 
        this, &DivePlanWindow::stopStepCellChanged);
}

void DivePlanWindow::refreshStopStepsTable() {
    if (m_isUpdating) {
        qDebug() << "Skipping refreshStopStepsTable() - already updating";
        return; // Prevent recursive calls
    }
    
    qDebug() << "Refreshing stop steps table - START - Count:" << m_divePlan->m_stopSteps.nbOfStopSteps();
    m_isUpdating = true;

    // Disconnect cell change signals temporarily
    disconnect(stopStepsTable, &QTableWidget::cellChanged, 
               this, &DivePlanWindow::stopStepCellChanged);
    
    // Clear and reset the table
    stopStepsTable->clearContents();
    stopStepsTable->setRowCount(m_divePlan->m_stopSteps.nbOfStopSteps());
    
    // Add stop steps to table
    for (int i = 0; i < m_divePlan->m_stopSteps.nbOfStopSteps(); ++i) {
        qDebug() << "  Adding stop step" << i << "to table: Depth ="
                 << m_divePlan->m_stopSteps.m_stopSteps[i].m_depth
                 << "Time =" << m_divePlan->m_stopSteps.m_stopSteps[i].m_time;
                 
        // Depth item
        QTableWidgetItem *depthItem = new QTableWidgetItem(
            QString::number(m_divePlan->m_stopSteps.m_stopSteps[i].m_depth, 'f', 1));
        depthItem->setTextAlignment(Qt::AlignCenter);
        stopStepsTable->setItem(i, STOP_COL_DEPTH, depthItem);
        
        // Time item
        QTableWidgetItem *timeItem = new QTableWidgetItem(
            QString::number(m_divePlan->m_stopSteps.m_stopSteps[i].m_time, 'f', 1));
        timeItem->setTextAlignment(Qt::AlignCenter);
        stopStepsTable->setItem(i, STOP_COL_TIME, timeItem);
        
        // Delete button (except for the last row if there's only one)
        if (m_divePlan->m_stopSteps.nbOfStopSteps() > 1 || i < m_divePlan->m_stopSteps.nbOfStopSteps() - 1) {
            stopStepsTable->setCellWidget(i, STOP_COL_DELETE, createDeleteButtonWidget(i).release());
        }
    }
    
    // Reconnect cell change signals
    connect(stopStepsTable, &QTableWidget::cellChanged, this, &DivePlanWindow::stopStepCellChanged);

    qDebug() << "Refreshing stop steps table - END";
    m_isUpdating = false;
}


} // namespace DiveComputer
