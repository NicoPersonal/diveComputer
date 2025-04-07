#include "gaslist_gui.hpp"

namespace DiveComputer {

// External global variables defined elsewhere
extern GasList g_gasList;
extern Parameters g_parameters;

// Column indices for better readability
enum GasColumns {
    COL_ACTIVE = 0,
    COL_TYPE = 1,
    COL_O2 = 2,
    COL_HE = 3,
    COL_MOD = 4,
    COL_END_NO_O2 = 5,
    COL_END_WITH_O2 = 6,
    COL_DENSITY = 7,
    COL_DELETE = 8,
    NUM_COLUMNS = 9
};

GasListWindow::GasListWindow(QWidget *parent) : QMainWindow(parent) {
    setWindowTitle("Gas Mixes");

    // Set up the UI first so content size is known
    setupUI();
    refreshGasTable();
    
    // Use the common window sizing and positioning function
    setWindowSizeAndPosition(this, preferredWidth, preferredHeight, WindowPosition::TOP_RIGHT);
}

void GasListWindow::setupUI() {
    // Create central widget
    QWidget *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);
    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);
    
    // Create top layout for gas selection controls
    QHBoxLayout *topLayout = new QHBoxLayout();
    
    // Add button at the left
    QPushButton *addButton = new QPushButton("+", this);
    addButton->setToolTip("Add Best Gas for Depth");
    addButton->setFixedWidth(30);
    topLayout->addWidget(addButton);
    
    // Add type dropdown aligned with type column
    bestGasTypeCombo = new QComboBox(this);
 
    bestGasTypeCombo->addItem("Bottom", static_cast<int>(DiveComputer::GasType::BOTTOM));
    bestGasTypeCombo->addItem("Deco", static_cast<int>(DiveComputer::GasType::DECO));
    bestGasTypeCombo->addItem("Diluent", static_cast<int>(DiveComputer::GasType::DILUENT));

    bestGasTypeCombo->setToolTip("Gas Type");
    topLayout->addWidget(bestGasTypeCombo);
    
    // Add "Depth:" label next to the depth input
    QLabel *depthLabel = new QLabel("Depth:", this);
    topLayout->addWidget(depthLabel);
    
    // Add depth input aligned with next column
    bestGasDepthEdit = new QLineEdit(this);
    bestGasDepthEdit->setPlaceholderText("Depth (m)");
    bestGasDepthEdit->setValidator(new QDoubleValidator(0, 200, 0, this));
    bestGasDepthEdit->setToolTip("Target Depth");
    bestGasDepthEdit->setFixedWidth(70);
    bestGasDepthEdit->setAlignment(Qt::AlignCenter);

    // Connect return key press to trigger add best gas
    connect(bestGasDepthEdit, &QLineEdit::returnPressed, this, &GasListWindow::addBestGas);
    topLayout->addWidget(bestGasDepthEdit);
    
    // Add spacer to push controls to the left
    topLayout->addStretch();
    
    // Connect add button to create best gas
    connect(addButton, &QPushButton::clicked, this, &GasListWindow::addBestGas);
    
    // Create gas table
    gasTable = new QTableWidget(0, NUM_COLUMNS, this);
    
    // Set table headers with units on second line
    QStringList headers;
    headers << "Active" << "Type" << "O₂\n%" << "He\n%" << "MOD\n(m)" << "END w/o O₂\n(m)" << "END w/ O₂\n(m)" << "Density\n(g/L)" << "";
    
    // Configure table using TableHelper
    TableHelper::configureTable(gasTable, QAbstractItemView::SelectItems);
    TableHelper::setHeaders(gasTable, headers);
    
    // Set specific column widths
    for (int i = 0; i < NUM_COLUMNS; i++) {
        if (i == COL_O2 || i == COL_HE || i == COL_MOD) {
            // Fixed width for O2 and He columns
            gasTable->setColumnWidth(i, 70);
        } else if (i == COL_END_NO_O2 || i == COL_END_WITH_O2) {
            // Fixed width for END columns
            gasTable->setColumnWidth(i, 70);
        } else if (i == COL_DENSITY) {
            // Fixed width for Density column
            gasTable->setColumnWidth(i, 70);
        } else if (i == COL_DELETE) {
            // Smaller width for delete column
            gasTable->setColumnWidth(i, 40);
        } else {
            // Adjust other columns automatically
            gasTable->horizontalHeader()->setSectionResizeMode(i, QHeaderView::ResizeToContents);
        }
    }
    
    // Add vertical separator line after He column
    gasTable->setStyleSheet(
        "QTableView::item:column(" + QString::number(COL_HE) + ") { border-right: 2px solid #888; }"
    );
    
    // Table cell change connections
    connect(gasTable, &QTableWidget::cellChanged, this, &GasListWindow::cellChanged);
    
    // Layout
    mainLayout->addLayout(topLayout);
    mainLayout->addWidget(gasTable);
}

void GasListWindow::refreshGasTable() {
    // Use the TableHelper for safe update
    TableHelper::safeUpdate(gasTable, this, &GasListWindow::cellChanged, [this]() {
        // Set row count
        gasTable->setRowCount(g_gasList.getGases().size());
        
        // Add gases to table
        for (size_t i = 0; i < g_gasList.getGases().size(); ++i) {
            addGasToTable(i, g_gasList.getGases()[i]);
        }
    });
    
    // Highlight END cells based on parameters
    highlightENDCells();
}

void GasListWindow::addGasToTable(int row, const Gas& gas) {
    // Active checkbox (centered in cell)
    QWidget* checkBoxWidget = new QWidget();
    QHBoxLayout* checkBoxLayout = new QHBoxLayout(checkBoxWidget);
    checkBoxLayout->setAlignment(Qt::AlignCenter);
    checkBoxLayout->setContentsMargins(0, 0, 0, 0);
    
    QCheckBox *activeCheckBox = new QCheckBox(this);
    activeCheckBox->setChecked(gas.m_gasStatus == GasStatus::ACTIVE);
    
    // Connect checkbox directly with lambda
    connect(activeCheckBox, &QCheckBox::checkStateChanged, [this, row](Qt::CheckState state) {
        gasStatusChanged(row, static_cast<int>(state));
    });

    checkBoxLayout->addWidget(activeCheckBox);
    gasTable->setCellWidget(row, COL_ACTIVE, checkBoxWidget);
    
    // Gas type combo box
    QComboBox *typeComboBox = new QComboBox(this);

    typeComboBox->addItem("Bottom", static_cast<int>(DiveComputer::GasType::BOTTOM));
    typeComboBox->addItem("Deco", static_cast<int>(DiveComputer::GasType::DECO));
    typeComboBox->addItem("Diluent", static_cast<int>(DiveComputer::GasType::DILUENT));

    typeComboBox->setCurrentIndex(static_cast<int>(gas.m_gasType));
    
    // Connect combobox with modern syntax
    connect(typeComboBox, &QComboBox::currentIndexChanged, this, &GasListWindow::onTypeComboBoxChanged);
    
    gasTable->setCellWidget(row, COL_TYPE, typeComboBox);
    
    // O2 percentage
    QTableWidgetItem *o2Item = TableHelper::createNumericCell(gas.m_o2Percent, 0, true);
    gasTable->setItem(row, COL_O2, o2Item);
    
    // He percentage
    QTableWidgetItem *heItem = TableHelper::createNumericCell(gas.m_hePercent, 0, true);
    gasTable->setItem(row, COL_HE, heItem);
    
    // MOD (calculated value, non-editable)
    QTableWidgetItem *modItem = TableHelper::createNumericCell(gas.m_MOD, 0, false);
    gasTable->setItem(row, COL_MOD, modItem);
    
    // END without O2 (calculated value, non-editable)
    QTableWidgetItem *endNoO2Item = TableHelper::createNumericCell(gas.ENDWithoutO2(gas.m_MOD), 0, false);
    gasTable->setItem(row, COL_END_NO_O2, endNoO2Item);
    
    // END with O2 (calculated value, non-editable)
    QTableWidgetItem *endWithO2Item = TableHelper::createNumericCell(gas.ENDWithO2(gas.m_MOD), 0, false);
    gasTable->setItem(row, COL_END_WITH_O2, endWithO2Item);
    
    // Gas density (calculated value, non-editable)
    QTableWidgetItem *densityItem = TableHelper::createNumericCell(gas.Density(gas.m_MOD), 1, false);
    gasTable->setItem(row, COL_DENSITY, densityItem);

    // Delete button
    gasTable->setCellWidget(row, COL_DELETE, createDeleteButtonWidget([this, row]() {
        deleteGas(row);
    }).release());
}

void GasListWindow::updateTableRow(int row) {
    if (row < 0 || row >= gasTable->rowCount()) {
        return;
    }
    
    const Gas& gas = g_gasList.getGases()[row];
    
    // MOD
    QTableWidgetItem* modItem = gasTable->item(row, COL_MOD);
    if (modItem) {
        modItem->setText(QString::number(gas.m_MOD, 'f', 0));
    }
    
    // END without O2
    QTableWidgetItem* endNoO2Item = gasTable->item(row, COL_END_NO_O2);
    if (endNoO2Item) {
        endNoO2Item->setText(QString::number(gas.ENDWithoutO2(gas.m_MOD), 'f', 0));
    }
    
    // END with O2
    QTableWidgetItem* endWithO2Item = gasTable->item(row, COL_END_WITH_O2);
    if (endWithO2Item) {
        endWithO2Item->setText(QString::number(gas.ENDWithO2(gas.m_MOD), 'f', 0));
    }
    
    // Density
    QTableWidgetItem* densityItem = gasTable->item(row, COL_DENSITY);
    if (densityItem) {
        densityItem->setText(QString::number(gas.Density(gas.m_MOD), 'f', 1));
    }
    
    // Highlight END cells
    highlightENDCells();
}

void GasListWindow::highlightENDCells() {
    const auto& gases = g_gasList.getGases();
    for (int row = 0; row < gasTable->rowCount(); ++row) {
        double endNoO2 = gases[row].ENDWithoutO2(gases[row].m_MOD);
        double endWithO2 = gases[row].ENDWithO2(gases[row].m_MOD);
        double density = gases[row].Density(gases[row].m_MOD);
        
        QTableWidgetItem* endNoO2Item = gasTable->item(row, COL_END_NO_O2);
        QTableWidgetItem* endWithO2Item = gasTable->item(row, COL_END_WITH_O2);
        QTableWidgetItem* densityItem = gasTable->item(row, COL_DENSITY);
        
        // Set warning backgrounds based on parameters
        if (g_parameters.m_defaultO2Narcotic) {
            // Highlight END with O2 if it exceeds the limit
            TableHelper::highlightCell(endWithO2Item, endWithO2 > g_parameters.m_defaultEnd);
        } else {
            // Highlight END without O2 if it exceeds the limit
            TableHelper::highlightCell(endNoO2Item, endNoO2 > g_parameters.m_defaultEnd);
        }
        
        // Highlight density if it exceeds the warning threshold
        TableHelper::highlightCell(densityItem, density > g_parameters.m_warningGasDensity);
    }
}

void GasListWindow::addNewGas() {
    // Add new gas with default values
    g_gasList.addGas(g_constants.m_oxygenInAir, 0.0, GasType::BOTTOM, GasStatus::ACTIVE);
    
    // Save gas list to file
    g_gasList.saveGaslistToFile();
    
    // Refresh the table
    refreshGasTable();
}

void GasListWindow::addBestGas() {
    // Get depth from input field
    double depth = bestGasDepthEdit->text().toDouble();
    
    // Get selected gas type
    GasType gasType = static_cast<GasType>(bestGasTypeCombo->currentIndex());
    
    // If depth is 0 or invalid, add default 21% O2 gas
    if (depth <= 0) {
        g_gasList.addGas(g_constants.m_oxygenInAir, 0.0, gasType, GasStatus::ACTIVE);
    } else {
        // Call bestGasForDepth and add the result
        Gas bestGas = Gas::bestGasForDepth(depth, gasType);
        g_gasList.addGas(bestGas.m_o2Percent, bestGas.m_hePercent, bestGas.m_gasType, GasStatus::ACTIVE);
    }
    
    // Save gas list to file
    g_gasList.saveGaslistToFile();
    
    // Refresh the table
    refreshGasTable();
}

void GasListWindow::deleteGas(int row) {
    if (row >= 0 && row < gasTable->rowCount()) {
        g_gasList.deleteGas(row);
        
        // Save gas list to file
        g_gasList.saveGaslistToFile();
        
        // Refresh the table
        refreshGasTable();
    }
}

void GasListWindow::cellChanged(int row, int column) {
    // Only handle editable columns
    if (column == COL_O2 || column == COL_HE) {
        // Update gas from row
        updateGasFromRow(row);
        
        // Update displayed values for this row
        updateTableRow(row);
        
        // Save changes
        g_gasList.saveGaslistToFile();
    }
}

void GasListWindow::gasTypeChanged(int row, int index) {
    // Convert index to GasType
    GasType gasType = static_cast<GasType>(index);
    
    // Update the gas in the list
    const auto& gases = g_gasList.getGases();
    if (row >= 0 && row < static_cast<int>(gases.size())) {
        double o2 = gases[row].m_o2Percent;
        double he = gases[row].m_hePercent;
        GasStatus status = gases[row].m_gasStatus;
        
        g_gasList.editGas(row, o2, he, gasType, status);
        
        // Update displayed values for this row
        updateTableRow(row);
        
        // Save changes
        g_gasList.saveGaslistToFile();
    }
}

void GasListWindow::gasStatusChanged(int row, int state) {
    // Convert state to GasStatus
    GasStatus gasStatus = (state == Qt::Checked) ? GasStatus::ACTIVE : GasStatus::INACTIVE;
    
    // Update the gas in the list
    const auto& gases = g_gasList.getGases();
    if (row >= 0 && row < static_cast<int>(gases.size())) {
        double o2 = gases[row].m_o2Percent;
        double he = gases[row].m_hePercent;
        GasType type = gases[row].m_gasType;
        
        g_gasList.editGas(row, o2, he, type, gasStatus);
        
        // Save changes
        g_gasList.saveGaslistToFile();
    }
}

void GasListWindow::updateGasFromRow(int row) {
    const auto& gases = g_gasList.getGases();
    if (row >= 0 && row < static_cast<int>(gases.size())) {
        // Get current values from table
        QTableWidgetItem* o2Item = gasTable->item(row, COL_O2);
        QTableWidgetItem* heItem = gasTable->item(row, COL_HE);
        QComboBox* typeCombo = qobject_cast<QComboBox*>(gasTable->cellWidget(row, COL_TYPE));
        
        // Find the checkbox within the container widget
        QWidget* checkBoxWidget = gasTable->cellWidget(row, COL_ACTIVE);
        QCheckBox* activeCheck = nullptr;
        if (checkBoxWidget) {
            // Find the checkbox within this widget
            activeCheck = checkBoxWidget->findChild<QCheckBox*>();
        }
        
        if (o2Item && heItem && typeCombo && activeCheck) {
            // Extract values
            double o2 = o2Item->text().toDouble();
            double he = heItem->text().toDouble();
            GasType type = static_cast<GasType>(typeCombo->currentIndex());
            GasStatus status = activeCheck->isChecked() ? GasStatus::ACTIVE : GasStatus::INACTIVE;
            
            // Update the gas
            g_gasList.editGas(row, o2, he, type, status);
        }
    }
}

void GasListWindow::onTypeComboBoxChanged(int index) {
    // Find which row this combobox belongs to
    QComboBox* senderComboBox = qobject_cast<QComboBox*>(sender());
    if (senderComboBox) {
        for (int row = 0; row < gasTable->rowCount(); ++row) {
            if (gasTable->cellWidget(row, COL_TYPE) == senderComboBox) {
                gasTypeChanged(row, index);
                break;
            }
        }
    }
}

void GasListWindow::onCheckboxChanged(int row) {
    QCheckBox* checkbox = qobject_cast<QCheckBox*>(sender());
    if (checkbox) {
        gasStatusChanged(row, checkbox->checkState());
    }
}

} // namespace DiveComputer

