// dive_plan_tables.cpp
#include "dive_plan_gui.hpp"
#include "main_gui.hpp"

namespace DiveComputer {


void DivePlanWindow::setupDivePlanTable() {
    // Set up column headers
    QStringList headers;
    headers << "Phase\n" << "Mode\n" << "Depth Range\n(m)" << "Time\n(min)" << "Run Time\n(min)"
        << "pAmb Max\n(bar)" << "pO2 Max\n(bar)" << "O2\n(%)" << "N2\n(%)" << "He\n(%)"
        << "GF\n(%)" << "GF Surf\n(%)" << "SAC\n(L/min)" << "Amb \n(L/min)" << "Step\n(L)"
        << "Density\n(g/L)" << "END -O2\n(m)" << "END +O2\n(m)" << "CNS\n(%)"
        << "CNS Multi\n(%)" << "OTU\n";
    divePlanTable->setHorizontalHeaderLabels(headers);
    
    // Configure table properties
    divePlanTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    divePlanTable->setSelectionMode(QAbstractItemView::SingleSelection);
    divePlanTable->setAlternatingRowColors(true);
    divePlanTable->verticalHeader()->setVisible(false);

    divePlanTable->setEditTriggers(QAbstractItemView::DoubleClicked | 
                                   QAbstractItemView::SelectedClicked | 
                                   QAbstractItemView::EditKeyPressed);
    
    // Initialize with fixed width columns
    divePlanTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    
    // Define column widths - using proportional values that will work well at any window size
    m_originalColumnWidths.clear();
    m_totalOriginalWidth = 0;
    
    // Set proportional column widths that will scale well
    // Values represent relative width proportions
    m_originalColumnWidths = {
        40,   // COL_PHASE - slightly wider for phase names
        40,   // COL_MODE
        100,  // COL_DEPTH_RANGE - needs more space for range format
        60,   // COL_TIME
        60,   // COL_RUN_TIME
        60,   // COL_PAMB_MAX
        44,   // COL_PO2_MAX
        44,   // COL_O2_PERCENT - percentage columns can be narrower
        44,   // COL_N2_PERCENT
        44,   // COL_HE_PERCENT
        44,   // COL_GF
        44,   // COL_GF_SURFACE
        44,   // COL_SAC_RATE
        44,   // COL_AMB_CONSUMPTION
        44,   // COL_STEP_CONSUMPTION
        44,   // COL_GAS_DENSITY
        44,   // COL_END_WO_O2
        44,   // COL_END_W_O2
        44,   // COL_CNS_SINGLE
        44,   // COL_CNS_MULTIPLE
        44    // COL_OTU
    };
    
    // Calculate total width for proportions
    for (int width : m_originalColumnWidths) {
        m_totalOriginalWidth += width;
    }
    
    // Mark columns as initialized
    m_columnsInitialized = true;
    
    // Adjust vertical header to fit content better
    divePlanTable->verticalHeader()->setDefaultSectionSize(
        divePlanTable->verticalHeader()->defaultSectionSize() * 1.2);
    
    // Trigger an immediate resize to set proper column widths
    QTimer::singleShot(0, this, &DivePlanWindow::resizeDivePlanTable);
    
    // Also attempt an immediate resize
    resizeDivePlanTable();

    // Hide N2 % column
    divePlanTable->setColumnHidden(COL_N2_PERCENT, true);
}

void DivePlanWindow::refreshDivePlanTable() {
    m_divePlan->calculate();

    // Disconnect cell change signals temporarily - use the safer pointer-to-member syntax
    disconnect(divePlanTable, &QTableWidget::cellChanged, this, &DivePlanWindow::divePlanCellChanged);

    // Block signals and disable updates to prevent intermediate redraws
    divePlanTable->blockSignals(true);
    divePlanTable->setUpdatesEnabled(false);
    
    // Clear and reset the table
    divePlanTable->clearContents();
    divePlanTable->setRowCount(m_divePlan->nbOfSteps());
    
    // Temporarily switch to fixed width mode during updates
    QHeaderView::ResizeMode oldMode = divePlanTable->horizontalHeader()->sectionResizeMode(0);
    divePlanTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    
    // Add dive steps to table
    for (int i = 0; i < m_divePlan->nbOfSteps(); ++i) {
        const DiveStep& step = m_divePlan->m_diveProfile[i];
        
        // Phase
        QTableWidgetItem *phaseItem = new QTableWidgetItem(getPhaseString(step.m_phase));
        phaseItem->setTextAlignment(Qt::AlignCenter);
        phaseItem->setFlags(phaseItem->flags() & ~Qt::ItemIsEditable); // Explicitly non-editable
        divePlanTable->setItem(i, COL_PHASE, phaseItem);
        
        // Mode
        QTableWidgetItem *modeItem = new QTableWidgetItem(getStepModeString(step.m_mode));
        modeItem->setTextAlignment(Qt::AlignCenter);
        modeItem->setFlags(modeItem->flags() & ~Qt::ItemIsEditable); // Explicitly non-editable
        divePlanTable->setItem(i, COL_MODE, modeItem);
        
        // Depth range
        QString depthRange = QString::number(step.m_startDepth, 'f', 0) + " → " +
                             QString::number(step.m_endDepth, 'f', 0);
        QTableWidgetItem *depthItem = new QTableWidgetItem(depthRange);
        depthItem->setTextAlignment(Qt::AlignCenter);
        depthItem->setFlags(depthItem->flags() & ~Qt::ItemIsEditable); // Explicitly non-editable
        divePlanTable->setItem(i, COL_DEPTH_RANGE, depthItem);
        
        // Time
        QTableWidgetItem *timeItem = new QTableWidgetItem(QString::number(step.m_time, 'f', 1));
        timeItem->setTextAlignment(Qt::AlignCenter);
        // Make only STOP phase time cells editable
        if (step.m_phase == Phase::STOP) {
            timeItem->setFlags(timeItem->flags() | Qt::ItemIsEditable);
        } else {
            timeItem->setFlags(timeItem->flags() & ~Qt::ItemIsEditable);
        }
        divePlanTable->setItem(i, COL_TIME, timeItem);
        
        // Run time
        QTableWidgetItem *runTimeItem = new QTableWidgetItem(QString::number(step.m_runTime, 'f', 1));
        runTimeItem->setTextAlignment(Qt::AlignCenter);
        runTimeItem->setFlags(runTimeItem->flags() & ~Qt::ItemIsEditable); // Explicitly non-editable
        divePlanTable->setItem(i, COL_RUN_TIME, runTimeItem);
        
        // pAmb Max
        QTableWidgetItem *pAmbItem = new QTableWidgetItem(QString::number(step.m_pAmbMax, 'f', 2));
        pAmbItem->setTextAlignment(Qt::AlignCenter);
        pAmbItem->setFlags(pAmbItem->flags() & ~Qt::ItemIsEditable); // Explicitly non-editable
        divePlanTable->setItem(i, COL_PAMB_MAX, pAmbItem);
        
        // pO2 Max
        QTableWidgetItem *pO2Item = new QTableWidgetItem(QString::number(step.m_pO2Max, 'f', 2));
        pO2Item->setTextAlignment(Qt::AlignCenter);
        pO2Item->setFlags(pO2Item->flags() & ~Qt::ItemIsEditable); // Explicitly non-editable
        divePlanTable->setItem(i, COL_PO2_MAX, pO2Item);
        
        // O2 %
        QTableWidgetItem *o2Item = new QTableWidgetItem(QString::number(step.m_o2Percent, 'f', 0));
        o2Item->setTextAlignment(Qt::AlignCenter);
        o2Item->setFlags(o2Item->flags() & ~Qt::ItemIsEditable); // Explicitly non-editable
        divePlanTable->setItem(i, COL_O2_PERCENT, o2Item);
        
        // N2 %
        QTableWidgetItem *n2Item = new QTableWidgetItem(QString::number(step.m_n2Percent, 'f', 0));
        n2Item->setTextAlignment(Qt::AlignCenter);
        n2Item->setFlags(n2Item->flags() & ~Qt::ItemIsEditable); // Explicitly non-editable
        divePlanTable->setItem(i, COL_N2_PERCENT, n2Item);
        
        // He %
        QTableWidgetItem *heItem = new QTableWidgetItem(QString::number(step.m_hePercent, 'f', 0));
        heItem->setTextAlignment(Qt::AlignCenter);
        heItem->setFlags(heItem->flags() & ~Qt::ItemIsEditable); // Explicitly non-editable
        divePlanTable->setItem(i, COL_HE_PERCENT, heItem);
        
        // GF
        QTableWidgetItem *gfItem = new QTableWidgetItem(QString::number(step.m_gf, 'f', 0));
        gfItem->setTextAlignment(Qt::AlignCenter);
        gfItem->setFlags(gfItem->flags() & ~Qt::ItemIsEditable); // Explicitly non-editable
        divePlanTable->setItem(i, COL_GF, gfItem);
        
        // GF Surface
        QTableWidgetItem *gfSurfaceItem = new QTableWidgetItem(QString::number(step.m_gfSurface, 'f', 0));
        gfSurfaceItem->setTextAlignment(Qt::AlignCenter);
        gfSurfaceItem->setFlags(gfSurfaceItem->flags() & ~Qt::ItemIsEditable); // Explicitly non-editable
        divePlanTable->setItem(i, COL_GF_SURFACE, gfSurfaceItem);
        
        // SAC Rate
        QTableWidgetItem *sacItem = new QTableWidgetItem(QString::number(step.m_sacRate, 'f', 0));
        sacItem->setTextAlignment(Qt::AlignCenter);
        sacItem->setFlags(sacItem->flags() & ~Qt::ItemIsEditable); // Explicitly non-editable
        divePlanTable->setItem(i, COL_SAC_RATE, sacItem);
        
        // Amb Consumption
        QTableWidgetItem *ambConsItem = new QTableWidgetItem(QString::number(step.m_ambConsumptionAtDepth, 'f', 0));
        ambConsItem->setTextAlignment(Qt::AlignCenter);
        ambConsItem->setFlags(ambConsItem->flags() & ~Qt::ItemIsEditable); // Explicitly non-editable
        divePlanTable->setItem(i, COL_AMB_CONSUMPTION, ambConsItem);
        
        // Step Consumption
        QTableWidgetItem *stepConsItem = new QTableWidgetItem(QString::number(step.m_stepConsumption, 'f', 0));
        stepConsItem->setTextAlignment(Qt::AlignCenter);
        stepConsItem->setFlags(stepConsItem->flags() & ~Qt::ItemIsEditable); // Explicitly non-editable
        divePlanTable->setItem(i, COL_STEP_CONSUMPTION, stepConsItem);
        
        // Gas Density
        QTableWidgetItem *densityItem = new QTableWidgetItem(QString::number(step.m_gasDensity, 'f', 1));
        densityItem->setTextAlignment(Qt::AlignCenter);
        densityItem->setFlags(densityItem->flags() & ~Qt::ItemIsEditable); // Explicitly non-editable
        divePlanTable->setItem(i, COL_GAS_DENSITY, densityItem);
        
        // END without O2
        QTableWidgetItem *endWoO2Item = new QTableWidgetItem(QString::number(step.m_endWithoutO2, 'f', 0));
        endWoO2Item->setTextAlignment(Qt::AlignCenter);
        endWoO2Item->setFlags(endWoO2Item->flags() & ~Qt::ItemIsEditable); // Explicitly non-editable
        divePlanTable->setItem(i, COL_END_WO_O2, endWoO2Item);
        
        // END with O2
        QTableWidgetItem *endWO2Item = new QTableWidgetItem(QString::number(step.m_endWithO2, 'f', 0));
        endWO2Item->setTextAlignment(Qt::AlignCenter);
        endWO2Item->setFlags(endWO2Item->flags() & ~Qt::ItemIsEditable); // Explicitly non-editable
        divePlanTable->setItem(i, COL_END_W_O2, endWO2Item);
        
        // CNS Single Dive
        QTableWidgetItem *cnsSingleItem = new QTableWidgetItem(QString::number(step.m_cnsTotalSingleDive, 'f', 0));
        cnsSingleItem->setTextAlignment(Qt::AlignCenter);
        cnsSingleItem->setFlags(cnsSingleItem->flags() & ~Qt::ItemIsEditable); // Explicitly non-editable
        divePlanTable->setItem(i, COL_CNS_SINGLE, cnsSingleItem);
        
        // CNS Multiple Dives
        QTableWidgetItem *cnsMultipleItem = new QTableWidgetItem(QString::number(step.m_cnsTotalMultipleDives, 'f', 0));
        cnsMultipleItem->setTextAlignment(Qt::AlignCenter);
        cnsMultipleItem->setFlags(cnsMultipleItem->flags() & ~Qt::ItemIsEditable); // Explicitly non-editable
        divePlanTable->setItem(i, COL_CNS_MULTIPLE, cnsMultipleItem);
        
        // OTU
        QTableWidgetItem *otuItem = new QTableWidgetItem(QString::number(step.m_otuTotal, 'f', 0));
        otuItem->setTextAlignment(Qt::AlignCenter);
        otuItem->setFlags(otuItem->flags() & ~Qt::ItemIsEditable); // Explicitly non-editable
        divePlanTable->setItem(i, COL_OTU, otuItem);
    }
    
    // Re-enable updates and signals
    divePlanTable->horizontalHeader()->setSectionResizeMode(oldMode);
    divePlanTable->setUpdatesEnabled(true);
    divePlanTable->blockSignals(false);
    
    // Highlight warning cells
    highlightWarningCells();

    // Reconnect cell change signals - use the safer pointer-to-member syntax
    connect(divePlanTable, &QTableWidget::cellChanged, this, &DivePlanWindow::divePlanCellChanged);

    // Ensure N2 column remains hidden
    divePlanTable->setColumnHidden(COL_N2_PERCENT, true);
}

void DivePlanWindow::resizeDivePlanTable() {
    // Skip if not initialized
    if (!divePlanTable || !m_columnsInitialized || m_totalOriginalWidth <= 0) {
        return;
    }
    
    // Force a layout update to ensure we have the correct width
    divePlanTable->updateGeometry();
    divePlanTable->parentWidget()->updateGeometry();
    QApplication::processEvents();
    
    // Calculate the available width (accounting for vertical scrollbar if visible)
    int scrollBarWidth = divePlanTable->verticalScrollBar()->isVisible() ? 
                         divePlanTable->verticalScrollBar()->width() : 0;
    int availableWidth = divePlanTable->width() - scrollBarWidth;
    
    // If table isn't visible or has no width, mark as dirty and return
    if (availableWidth <= 0 || !divePlanTable->isVisible()) {
        m_tableDirty = true;
        return;
    }
    
    // Temporarily disable updates to prevent flickering
    divePlanTable->setUpdatesEnabled(false);
    
    // Set section resize mode to fixed to allow manual column width adjustment
    divePlanTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    
    // Count visible columns and calculate visible proportion total
    double totalVisibleProportion = 0.0;
    int visibleColumnCount = 0;
    
    for (int i = 0; i < divePlanTable->horizontalHeader()->count(); ++i) {
        if (!divePlanTable->isColumnHidden(i)) {
            totalVisibleProportion += m_originalColumnWidths[i];
            visibleColumnCount++;
        }
    }
    
    if (visibleColumnCount == 0) {
        divePlanTable->setUpdatesEnabled(true);
        return;
    }
    
    // Calculate proportional widths
    QVector<int> columnWidths(divePlanTable->horizontalHeader()->count(), 0);
    int totalCalculatedWidth = 0;
    
    for (int i = 0; i < divePlanTable->horizontalHeader()->count(); ++i) {
        if (divePlanTable->isColumnHidden(i)) {
            columnWidths[i] = 0;
        } else {
            double proportion = static_cast<double>(m_originalColumnWidths[i]) / totalVisibleProportion;
            int colWidth = static_cast<int>(availableWidth * proportion);
            // Ensure minimum width
            columnWidths[i] = qMax(10, colWidth);
            totalCalculatedWidth += columnWidths[i];
        }
    }
    
    // Handle any rounding errors by adjusting the last visible column
    int lastVisibleCol = divePlanTable->horizontalHeader()->count() - 1;
    while (lastVisibleCol >= 0 && divePlanTable->isColumnHidden(lastVisibleCol)) {
        lastVisibleCol--;
    }
    
    if (lastVisibleCol >= 0) {
        // Adjust the last column to take up any remaining space
        int widthDifference = availableWidth - totalCalculatedWidth;
        columnWidths[lastVisibleCol] = qMax(10, columnWidths[lastVisibleCol] + widthDifference);
    }
    
    // Apply calculated widths
    for (int i = 0; i < divePlanTable->horizontalHeader()->count(); ++i) {
        if (!divePlanTable->isColumnHidden(i)) {
            divePlanTable->setColumnWidth(i, columnWidths[i]);
        }
    }
    
    // Re-enable updates
    divePlanTable->setUpdatesEnabled(true);
    m_tableDirty = false;
}

void DivePlanWindow::divePlanCellChanged(int row, int column)
{
    // Only handle time column for STOP phases
    if (column == COL_TIME) {
        const DiveStep& step = m_divePlan->m_diveProfile[row];
        
        // Check if this is a STOP phase
        if (step.m_phase == Phase::STOP) {
            // Get the updated value
            QTableWidgetItem* item = divePlanTable->item(row, column);
            if (!item) return;
            
            bool ok;
            double newTime = item->text().toDouble(&ok);
            
            if (ok) {
                // Find the corresponding stop step
                for (int i = 0; i < m_divePlan->m_stopSteps.nbOfStopSteps(); ++i) {
                    // Match by depth to find the corresponding stop step
                    if (std::abs(m_divePlan->m_stopSteps.m_stopSteps[i].m_depth - step.m_startDepth) < 0.1) {
                        // Update the stop step time
                        m_divePlan->m_stopSteps.editStopStep(i, 
                                                         m_divePlan->m_stopSteps.m_stopSteps[i].m_depth, 
                                                         newTime);
                        
                        // Rebuild everything
                        rebuildDivePlan();
                        refreshDivePlan();
                        
                        // Allow UI to process events
                        QApplication::processEvents();
                        
                        break;
                    }
                }
            }
        }
    }
}



} // namespace DiveComputer