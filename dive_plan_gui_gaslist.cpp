#include "dive_plan_gui.hpp"
#include "main_gui.hpp"
#include <qtheaders.hpp>

namespace DiveComputer {

void DivePlanWindow::setupGasesTable() {
    // Set up column headers
    QStringList headers;
    headers << "O2\n(%)" << "He\n(%)" << "Switch\n(m)" << "Switch\n(ppO2)" << "Consumption\n(L)" 
            << "Tanks\n(#)" << "Capacity\n(L)" << "Fill\n(bar)" << "Reserve\n(bar)" << "End\n(bar)";
    
    // Configure table
    TableHelper::configureTable(gasesTable, QAbstractItemView::SelectRows);
    TableHelper::setHeaders(gasesTable, headers);
    
    // Set edit triggers
    gasesTable->setEditTriggers(QAbstractItemView::DoubleClicked | 
                               QAbstractItemView::SelectedClicked | 
                               QAbstractItemView::EditKeyPressed);
    
    // Initialize with fixed width columns
    gasesTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    
    // Define column widths using proportional values
    m_gasesColumnWidths.clear();
    m_totalGasesWidth = 0;
    
    // Set proportional column widths that will scale well
    // Values represent relative width proportions
    m_gasesColumnWidths = {
        40,   // GAS_COL_O2 - narrow for percentage
        40,   // GAS_COL_HE - narrow for percentage
        50,   // GAS_COL_SWITCH_DEPTH - same width as previous MOD
        50,   // GAS_COL_SWITCH_PPO2 - same width as previous MOD
        90,   // GAS_COL_CONSUMPTION - wider for consumption values
        40,   // GAS_COL_NB_TANKS
        60,   // GAS_COL_TANK_CAPACITY
        50,   // GAS_COL_FILLING_PRESSURE
        60,   // GAS_COL_RESERVE_PRESSURE
        50    // GAS_COL_END_PRESSURE
    };
    
    // Calculate total width for proportions
    for (int width : m_gasesColumnWidths) {
        m_totalGasesWidth += width;
    }
    
    // Mark columns as initialized
    m_gasesColumnsInitialized = true;
    
    // Connect cell change signal
    connect(gasesTable, &QTableWidget::cellChanged, this, &DivePlanWindow::gasTableCellChanged);
        
    // Set reasonable default widths directly
    for (int i = 0; i < gasesTable->horizontalHeader()->count(); ++i) {
        gasesTable->setColumnWidth(i, m_gasesColumnWidths[i]);
    }
}

void DivePlanWindow::refreshGasesTable() {
    static bool isRefreshing = false;
    if (isRefreshing) return;
    
    isRefreshing = true;
    
    // Use the TableHelper for safe update
    TableHelper::safeUpdate(gasesTable, this, &DivePlanWindow::gasTableCellChanged, [this]() {
        // Get gases sorted by increasing O2 content
        std::vector<GasAvailable> sortedGases = m_divePlan->m_gasAvailable;
        std::sort(sortedGases.begin(), sortedGases.end(), 
                [](const GasAvailable& a, const GasAvailable& b) {
                    return a.m_gas.m_o2Percent < b.m_gas.m_o2Percent;
                });
        
        // Clear and rebuild the mapping
        m_gasRowToOriginalIndex.clear();
        
        // Create mapping from table rows to original gas indices
        for (size_t i = 0; i < sortedGases.size(); ++i) {
            // Find original index in unsorted list
            for (size_t j = 0; j < m_divePlan->m_gasAvailable.size(); ++j) {
                if (std::abs(m_divePlan->m_gasAvailable[j].m_gas.m_o2Percent - sortedGases[i].m_gas.m_o2Percent) < 0.1 &&
                    std::abs(m_divePlan->m_gasAvailable[j].m_gas.m_hePercent - sortedGases[i].m_gas.m_hePercent) < 0.1) {
                    m_gasRowToOriginalIndex.push_back(j);
                    break;
                }
            }
        }
        
        // Set number of rows
        gasesTable->setRowCount(sortedGases.size());
        
        // Add gases to table
        for (size_t i = 0; i < sortedGases.size(); ++i) {
            const GasAvailable& gas = sortedGases[i];
            
            // O2 Percent
            QTableWidgetItem *o2Item = TableHelper::createNumericCell(gas.m_gas.m_o2Percent, 0, false);
            o2Item->setData(Qt::UserRole, QVariant(m_gasRowToOriginalIndex[i])); // Store original index
            gasesTable->setItem(i, GAS_COL_O2, o2Item);
            
            // He Percent
            gasesTable->setItem(i, GAS_COL_HE, 
                TableHelper::createNumericCell(gas.m_gas.m_hePercent, 0, false));
            
            // Switch Depth (m)
            gasesTable->setItem(i, GAS_COL_SWITCH_DEPTH, 
                TableHelper::createNumericCell(gas.m_switchDepth, 0, false));
            
            // Switch PpO2
            gasesTable->setItem(i, GAS_COL_SWITCH_PPO2, 
                TableHelper::createNumericCell(gas.m_switchPpO2, 2, false));
            
            // Consumption
            gasesTable->setItem(i, GAS_COL_CONSUMPTION, 
                TableHelper::createNumericCell(gas.m_consumption, 0, false));
            
            // Number of Tanks - editable
            QTableWidgetItem *nbTanksItem = TableHelper::createNumericCell(gas.m_nbTanks, 0, true);
            gasesTable->setItem(i, GAS_COL_NB_TANKS, nbTanksItem);
            
            // Tank Capacity - editable
            QTableWidgetItem *capacityItem = TableHelper::createNumericCell(gas.m_tankCapacity, 1, true);
            gasesTable->setItem(i, GAS_COL_TANK_CAPACITY, capacityItem);
            
            // Filling Pressure - editable
            QTableWidgetItem *fillingItem = TableHelper::createNumericCell(gas.m_fillingPressure, 0, true);
            gasesTable->setItem(i, GAS_COL_FILLING_PRESSURE, fillingItem);
            
            // Reserve Pressure - editable
            QTableWidgetItem *reserveItem = TableHelper::createNumericCell(gas.m_reservePressure, 0, true);
            gasesTable->setItem(i, GAS_COL_RESERVE_PRESSURE, reserveItem);
            
            // End Pressure
            QTableWidgetItem *endItem = TableHelper::createNumericCell(gas.m_endPressure, 0, false);
            
            // Highlight based on pressure status
            if (gas.m_endPressure <= 0) {
                // Flashy red for out of gas - white text gives better contrast
                endItem->setBackground(QBrush(QColor(255, 0, 0)));
                endItem->setForeground(QBrush(QColor(255, 255, 255)));
            } else if (gas.m_endPressure <= gas.m_reservePressure) {
                // Light red for low gas (at or below reserve)
                endItem->setBackground(QBrush(QColor(255, 200, 200)));
            }
            
            gasesTable->setItem(i, GAS_COL_END_PRESSURE, endItem);
        }
    });
    
    // Resize table columns
    resizeGasesTable();

    isRefreshing = false;
}

void DivePlanWindow::resizeGasesTable() {
    if (!gasesTable || !m_gasesColumnsInitialized || m_totalGasesWidth <= 0) {
        return;
    }
    
    // Get the width of the table itself
    QWidget* parentWidget = gasesTable->parentWidget();
    if (!parentWidget) return;
    
    // Force layout to update
    gasesTable->updateGeometry();
    parentWidget->updateGeometry();
    QApplication::processEvents();
        
    // Account for table widget's internal margins, frame, etc.
    // Using the actual table viewport width gives us the true available space for columns
    int availableWidth = gasesTable->viewport()->width() - 5; // 5px safety margin
    
    // If the table isn't yet properly sized, estimate from parent
    if (availableWidth <= 50) {
        availableWidth = parentWidget->width() - 30; // Approximate margins and scrollbar
    }
    
    // Disable updates while resizing
    gasesTable->setUpdatesEnabled(false);
    
    // Scale all columns proportionally based on the available width
    double scaleFactor = static_cast<double>(availableWidth) / m_totalGasesWidth;
    int totalActualWidth = 0;
    
    // First pass: resize all columns according to their proportions
    for (int i = 0; i < gasesTable->horizontalHeader()->count(); ++i) {
        int scaledWidth = qMax(15, static_cast<int>(m_gasesColumnWidths[i] * scaleFactor));
        
        // For all but the last column
        if (i < gasesTable->horizontalHeader()->count() - 1) {
            gasesTable->setColumnWidth(i, scaledWidth);
            totalActualWidth += scaledWidth;
        }
    }
    
    // Give any remaining space to the last column
    int lastColumnWidth = qMax(15, availableWidth - totalActualWidth);
    gasesTable->setColumnWidth(gasesTable->horizontalHeader()->count() - 1, lastColumnWidth);
    
    gasesTable->setUpdatesEnabled(true);
}

void DivePlanWindow::updateGasTablePressures() {
    // Block signals during the update
    bool wasBlocked = gasesTable->blockSignals(true);

    // Get current values from the table
    for (int row = 0; row < gasesTable->rowCount(); ++row) {
        // Get the original index from the O2 column's user data
        QTableWidgetItem* o2Item = gasesTable->item(row, GAS_COL_O2);
        if (!o2Item) continue;
        
        int originalIndex = o2Item->data(Qt::UserRole).toInt();
        
        // Make sure the index is valid
        if (originalIndex >= 0 && originalIndex < static_cast<int>(m_divePlan->m_gasAvailable.size())) {
            // Get the gas at the original index
            GasAvailable& gas = m_divePlan->m_gasAvailable[originalIndex];
            
            // Calculate how much gas is available in total
            double totalCapacity = gas.m_nbTanks * gas.m_tankCapacity * gas.m_fillingPressure;
            
            // End pressure = (total capacity - consumption) / (nb tanks * tank capacity)
            double endPressure = (totalCapacity - gas.m_consumption) / (gas.m_nbTanks * gas.m_tankCapacity);
            
            // Update the gas object
            gas.m_endPressure = endPressure;
            
            // Update the end pressure cell
            QTableWidgetItem* endItem = gasesTable->item(row, GAS_COL_END_PRESSURE);
            if (endItem) {
                endItem->setText(QString::number(endPressure, 'f', 0));
                
                // Use TableHelper to highlight the cell
                if (endPressure <= 0) {
                    // Flashy red for out of gas - white text for contrast
                    endItem->setBackground(QBrush(QColor(255, 0, 0)));
                    endItem->setForeground(QBrush(QColor(255, 255, 255)));
                } else if (endPressure <= gas.m_reservePressure) {
                    // Light red for low gas (at or below reserve)
                    endItem->setBackground(QBrush(QColor(255, 200, 200)));
                    // Use default text color
                } else {
                    // Return to default styling
                    endItem->setBackground(QBrush());
                    // Reset foreground if it was white
                    if (endItem->foreground().color() == QColor(255, 255, 255)) {
                        endItem->setForeground(QBrush());
                    }
                }
            }
        }
    }
    gasesTable->blockSignals(wasBlocked);
}

void DivePlanWindow::gasTableCellChanged(int row, int column) {
    // Only handle editable columns
    if (column == GAS_COL_NB_TANKS || 
        column == GAS_COL_TANK_CAPACITY || 
        column == GAS_COL_FILLING_PRESSURE || 
        column == GAS_COL_RESERVE_PRESSURE ||
        column == GAS_COL_SWITCH_DEPTH ||
        column == GAS_COL_SWITCH_PPO2) {
        
        // Get the new value
        QTableWidgetItem* item = gasesTable->item(row, column);
        if (!item) return;
        
        double newValue = 0.0;
        QString fieldName;
        double minValue = 0.0;
        double maxValue = 1000.0;
        
        // Set field-specific validation parameters
        switch (column) {
            case GAS_COL_NB_TANKS:
                fieldName = "Number of Tanks";
                minValue = 1;
                maxValue = 20;
                break;
            case GAS_COL_TANK_CAPACITY:
                fieldName = "Tank Capacity";
                minValue = 1.0;
                maxValue = 50.0;
                break;
            case GAS_COL_FILLING_PRESSURE:
                fieldName = "Filling Pressure";
                minValue = 50.0;
                maxValue = 300.0;
                break;
            case GAS_COL_RESERVE_PRESSURE:
                fieldName = "Reserve Pressure";
                minValue = 10.0;
                maxValue = 100.0;
                break;
            case GAS_COL_SWITCH_DEPTH:
                fieldName = "Switch Depth";
                minValue = 0.0;
                maxValue = 200.0;
                break;
            case GAS_COL_SWITCH_PPO2:
                fieldName = "Switch ppO2";
                minValue = 0.1;
                maxValue = 2.0;
                break;
        }
        
        // Validate the input
        if (ErrorHandler::validateNumericInput(item->text(), newValue, minValue, maxValue, fieldName)) {
            // Get the original index of the gas from the O2 column's user data
            QTableWidgetItem* o2Item = gasesTable->item(row, GAS_COL_O2);
            if (!o2Item) return;
            
            int originalIndex = o2Item->data(Qt::UserRole).toInt();
            
            // Make sure the index is valid
            if (originalIndex >= 0 && originalIndex < static_cast<int>(m_divePlan->m_gasAvailable.size())) {
                // Get the gas at the original index
                GasAvailable& gas = m_divePlan->m_gasAvailable[originalIndex];
                
                // Update the appropriate property
                switch (column) {
                    case GAS_COL_SWITCH_DEPTH:
                        gas.m_switchDepth = newValue;
                        break;
                    case GAS_COL_SWITCH_PPO2:
                        gas.m_switchPpO2 = newValue;
                        break;
                    case GAS_COL_NB_TANKS:
                        gas.m_nbTanks = static_cast<int>(newValue);
                        break;
                    case GAS_COL_TANK_CAPACITY:
                        gas.m_tankCapacity = newValue;
                        break;
                    case GAS_COL_FILLING_PRESSURE:
                        gas.m_fillingPressure = newValue;
                        break;
                    case GAS_COL_RESERVE_PRESSURE:
                        gas.m_reservePressure = newValue;
                        // Update highlighting if needed
                        QTableWidgetItem* endItem = gasesTable->item(row, GAS_COL_END_PRESSURE);
                        if (endItem) {
                            double endPressure = 0.0;
                            if (ErrorHandler::validateNumericInput(endItem->text(), endPressure, 0.0, 1000.0, "End Pressure", false)) {
                                if (endPressure <= newValue && endPressure > 0) {
                                    endItem->setBackground(QBrush(QColor(255, 200, 200)));
                                } else if (endPressure > newValue) {
                                    endItem->setBackground(QBrush());
                                }
                            }
                        }
                        break;
                }
                
                // Update the gas table with new end pressures
                updateGasTablePressures();
            }
        } else {
            // Revert to previous value if validation fails
            QTableWidgetItem* o2Item = gasesTable->item(row, GAS_COL_O2);
            if (!o2Item) return;
            
            int originalIndex = o2Item->data(Qt::UserRole).toInt();
            
            if (originalIndex >= 0 && originalIndex < static_cast<int>(m_divePlan->m_gasAvailable.size())) {
                GasAvailable& gas = m_divePlan->m_gasAvailable[originalIndex];
                
                // Reset to original value
                gasesTable->blockSignals(true);
                switch (column) {
                    case GAS_COL_SWITCH_DEPTH:
                        item->setText(QString::number(gas.m_switchDepth, 'f', 0));
                        break;
                    case GAS_COL_SWITCH_PPO2:
                        item->setText(QString::number(gas.m_switchPpO2, 'f', 2));
                        break;
                    case GAS_COL_NB_TANKS:
                        item->setText(QString::number(gas.m_nbTanks));
                        break;
                    case GAS_COL_TANK_CAPACITY:
                        item->setText(QString::number(gas.m_tankCapacity, 'f', 1));
                        break;
                    case GAS_COL_FILLING_PRESSURE:
                        item->setText(QString::number(gas.m_fillingPressure, 'f', 0));
                        break;
                    case GAS_COL_RESERVE_PRESSURE:
                        item->setText(QString::number(gas.m_reservePressure, 'f', 0));
                        break;
                }
                gasesTable->blockSignals(false);
            }
        }
    }
}





} // namespace DiveComputer