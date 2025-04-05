#include "gaslist_gui.hpp"
#include "enum.hpp"
#include <QVariant>

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
    connect(bestGasDepthEdit, SIGNAL(returnPressed()), this, SLOT(addBestGas()));
    topLayout->addWidget(bestGasDepthEdit);
    
    // Add spacer to push controls to the left
    topLayout->addStretch();
    
    // Connect add button to create best gas
    connect(addButton, SIGNAL(clicked()), this, SLOT(addBestGas()));
    
    // Create gas table
    gasTable = new QTableWidget(0, NUM_COLUMNS, this);
    
    // Set table headers with units on second line
    QStringList headers;
    headers << "Active" << "Type" << "O₂\n%" << "He\n%" << "MOD\n(m)" << "END w/o O₂\n(m)" << "END w/ O₂\n(m)" << "Density\n(g/L)" << "";
    gasTable->setHorizontalHeaderLabels(headers);
    
    // Configure table
    gasTable->setSelectionBehavior(QAbstractItemView::SelectItems); // Changed to allow selecting specific items
    gasTable->setSelectionMode(QAbstractItemView::SingleSelection); // Only select one item at a time
    gasTable->setAlternatingRowColors(true);
    gasTable->verticalHeader()->setVisible(false);
    
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
    // Qt doesn't have direct support for column separators, so we'll customize the table's stylesheet
    gasTable->setStyleSheet(
        "QTableView::item:column(" + QString::number(COL_HE) + ") { border-right: 2px solid #888; }"
    );
    
    gasTable->setEditTriggers(QAbstractItemView::DoubleClicked | QAbstractItemView::SelectedClicked | QAbstractItemView::EditKeyPressed);
    
    // Table cell change connections
    connect(gasTable, SIGNAL(cellChanged(int, int)), this, SLOT(cellChanged(int, int)));
    
    // Layout
    mainLayout->addLayout(topLayout);
    mainLayout->addWidget(gasTable);
    
    // Note: We no longer set a size here, it's done in the constructor
}

void GasListWindow::refreshGasTable() {
    // Disconnect while updating
    disconnect(gasTable, &QTableWidget::cellChanged, this, &GasListWindow::cellChanged);
    
    // Clear and reset the table
    gasTable->clearContents();
    gasTable->setRowCount(g_gasList.gases.size());
    
    // Add gases to table
    for (size_t i = 0; i < g_gasList.gases.size(); ++i) {
        addGasToTable(i, g_gasList.gases[i]);
    }
    
    // Reconnect signals
    connect(gasTable, SIGNAL(cellChanged(int, int)), this, SLOT(cellChanged(int, int)));
    
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
    
    // Connect checkbox
    connect(activeCheckBox, &QCheckBox::stateChanged, [this, row](int state) {
        gasStatusChanged(row, state);
    });

    checkBoxLayout->addWidget(activeCheckBox);
    gasTable->setCellWidget(row, COL_ACTIVE, checkBoxWidget);
    
    // Gas type combo box
    QComboBox *typeComboBox = new QComboBox(this);

    typeComboBox->addItem("Bottom", static_cast<int>(DiveComputer::GasType::BOTTOM));
    typeComboBox->addItem("Deco", static_cast<int>(DiveComputer::GasType::DECO));
    typeComboBox->addItem("Diluent", static_cast<int>(DiveComputer::GasType::DILUENT));

    typeComboBox->setCurrentIndex(static_cast<int>(gas.m_gasType));
    
    // Connect combobox
    connect(typeComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(onTypeComboBoxChanged(int)));
    
    gasTable->setCellWidget(row, COL_TYPE, typeComboBox);
    
    // O2 percentage
    QTableWidgetItem *o2Item = new QTableWidgetItem(QString::number(gas.m_o2Percent, 'f', 0));
    o2Item->setTextAlignment(Qt::AlignCenter);
    gasTable->setItem(row, COL_O2, o2Item);
    
    // He percentage
    QTableWidgetItem *heItem = new QTableWidgetItem(QString::number(gas.m_hePercent, 'f', 0));
    heItem->setTextAlignment(Qt::AlignCenter);
    gasTable->setItem(row, COL_HE, heItem);
    
    // MOD (calculated value, non-editable)
    QTableWidgetItem *modItem = new QTableWidgetItem(QString::number(gas.m_MOD, 'f', 0));
    modItem->setTextAlignment(Qt::AlignCenter);
    modItem->setFlags(modItem->flags() & ~Qt::ItemIsEditable & ~Qt::ItemIsSelectable);
    gasTable->setItem(row, COL_MOD, modItem);
    
    // END without O2 (calculated value, non-editable)
    QTableWidgetItem *endNoO2Item = new QTableWidgetItem(QString::number(gas.ENDWithoutO2(gas.m_MOD), 'f', 0));
    endNoO2Item->setTextAlignment(Qt::AlignCenter);
    endNoO2Item->setFlags(endNoO2Item->flags() & ~Qt::ItemIsEditable & ~Qt::ItemIsSelectable);
    gasTable->setItem(row, COL_END_NO_O2, endNoO2Item);
    
    // END with O2 (calculated value, non-editable)
    QTableWidgetItem *endWithO2Item = new QTableWidgetItem(QString::number(gas.ENDWithO2(gas.m_MOD), 'f', 0));
    endWithO2Item->setTextAlignment(Qt::AlignCenter);
    endWithO2Item->setFlags(endWithO2Item->flags() & ~Qt::ItemIsEditable & ~Qt::ItemIsSelectable);
    gasTable->setItem(row, COL_END_WITH_O2, endWithO2Item);
    
    // Gas density (calculated value, non-editable)
    QTableWidgetItem *densityItem = new QTableWidgetItem(QString::number(gas.Density(gas.m_MOD), 'f', 1));
    densityItem->setTextAlignment(Qt::AlignCenter);
    densityItem->setFlags(densityItem->flags() & ~Qt::ItemIsEditable & ~Qt::ItemIsSelectable);
    gasTable->setItem(row, COL_DENSITY, densityItem);
    
    // Delete button
    gasTable->setCellWidget(row, COL_DELETE, createDeleteButtonWidget(row));
}

void GasListWindow::updateTableRow(int row) {
    if (row < 0 || row >= gasTable->rowCount()) {
        return;
    }
    
    const Gas& gas = g_gasList.gases[row];
    
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
    for (int row = 0; row < gasTable->rowCount(); ++row) {
        double endNoO2 = g_gasList.gases[row].ENDWithoutO2(g_gasList.gases[row].m_MOD);
        double endWithO2 = g_gasList.gases[row].ENDWithO2(g_gasList.gases[row].m_MOD);
        double density = g_gasList.gases[row].Density(g_gasList.gases[row].m_MOD);
        
        QTableWidgetItem* endNoO2Item = gasTable->item(row, COL_END_NO_O2);
        QTableWidgetItem* endWithO2Item = gasTable->item(row, COL_END_WITH_O2);
        QTableWidgetItem* densityItem = gasTable->item(row, COL_DENSITY);
        
        // Reset background colors
        if (endNoO2Item) {
            endNoO2Item->setBackground(QBrush());
        }
        
        if (endWithO2Item) {
            endWithO2Item->setBackground(QBrush());
        }
        
        if (densityItem) {
            densityItem->setBackground(QBrush());
        }
        
        // Set warning backgrounds based on parameters
        if (g_parameters.m_defaultO2Narcotic) {
            // Highlight END with O2 if it exceeds the limit
            if (endWithO2Item && endWithO2 > g_parameters.m_defaultEnd) {
                endWithO2Item->setBackground(QBrush(QColor(255, 200, 200)));
            }
        } else {
            // Highlight END without O2 if it exceeds the limit
            if (endNoO2Item && endNoO2 > g_parameters.m_defaultEnd) {
                endNoO2Item->setBackground(QBrush(QColor(255, 200, 200)));
            }
        }
        
        // Highlight density if it exceeds the warning threshold
        if (densityItem && density > g_parameters.m_warningGasDensity) {
            densityItem->setBackground(QBrush(QColor(255, 200, 200)));
        }
    }
}

QWidget* GasListWindow::createDeleteButtonWidget(int row) {
    QWidget* widget = new QWidget();
    QHBoxLayout* layout = new QHBoxLayout(widget);
    layout->setAlignment(Qt::AlignCenter);
    layout->setContentsMargins(2, 2, 2, 2);
    
    QPushButton* deleteButton = new QPushButton("×");
    deleteButton->setFixedSize(20, 20);
    deleteButton->setToolTip("Delete this gas");
    
    connect(deleteButton, &QPushButton::clicked, [this, row]() {
        deleteGas(row);
    });
    
    layout->addWidget(deleteButton);
    return widget;
}

void GasListWindow::addNewGas() {
    // Add new gas with default values
    g_gasList.addGas(21.0, 0.0, GasType::BOTTOM, GasStatus::ACTIVE);
    
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
        g_gasList.addGas(21.0, 0.0, gasType, GasStatus::ACTIVE);
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
    if (row >= 0 && row < static_cast<int>(g_gasList.gases.size())) {
        double o2 = g_gasList.gases[row].m_o2Percent;
        double he = g_gasList.gases[row].m_hePercent;
        GasStatus status = g_gasList.gases[row].m_gasStatus;
        
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
    if (row >= 0 && row < static_cast<int>(g_gasList.gases.size())) {
        double o2 = g_gasList.gases[row].m_o2Percent;
        double he = g_gasList.gases[row].m_hePercent;
        GasType type = g_gasList.gases[row].m_gasType;
        
        g_gasList.editGas(row, o2, he, type, gasStatus);
        
        // Save changes
        g_gasList.saveGaslistToFile();
    }
}

void GasListWindow::updateGasFromRow(int row) {
    if (row >= 0 && row < static_cast<int>(g_gasList.gases.size())) {
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