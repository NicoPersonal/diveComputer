#ifndef GASLIST_GUI_HPP
#define GASLIST_GUI_HPP

#include "qtheaders.hpp"
#include "gaslist.hpp"
#include "gas.hpp"
#include "parameters.hpp"
#include "enum.hpp"
#include "ui_utils.hpp"
#include "table_helper.hpp"

namespace DiveComputer {

// Gas List Editor Window class
class GasListWindow : public QMainWindow {
    Q_OBJECT
    
public:
    GasListWindow(QWidget *parent = nullptr);
    
private:
    // Window size
    const int preferredWidth = 630;
    const int preferredHeight = 400;

    // UI elements
    QTableWidget *gasTable;
    QComboBox *bestGasTypeCombo;
    QLineEdit *bestGasDepthEdit;
    
    // Setup functions
    void setupUI();
    void refreshGasTable();
    void addGasToTable(int row, const Gas& gas);
    void updateTableRow(int row);
    void highlightENDCells();
    void setupTableRowConnections(int row);
    
    // Helper functions
    void updateGasFromRow(int row);

private slots:
    void addNewGas();
    void addBestGas();
    void deleteGas(int row);
    void cellChanged(int row, int column);
    void gasTypeChanged(int row, int index);
    void gasStatusChanged(int row, int state);
    void onTypeComboBoxChanged(int index);
    void onCheckboxChanged(int row);
};

} // namespace DiveComputer

#endif // GASLIST_GUI_HPP