#ifndef DIVE_PLAN_GUI_HPP
#define DIVE_PLAN_GUI_HPP

#include <QElapsedTimer>
#include "qtheaders.hpp"
#include "dive_plan.hpp"
#include "parameters.hpp"
#include "enum.hpp"
#include "global.hpp"
#include "ui_utils.hpp"

// Forward declaration of MainWindow class
namespace DiveComputer { class MainWindow; }

namespace DiveComputer {

// Column indices for tables
enum DivePlanColumns {
    COL_PHASE = 0,
    COL_MODE = 1,
    COL_DEPTH_RANGE = 2,
    COL_TIME = 3,
    COL_RUN_TIME = 4,
    COL_PAMB_MAX = 5,
    COL_PO2_MAX = 6,
    COL_O2_PERCENT = 7,
    COL_N2_PERCENT = 8,
    COL_HE_PERCENT = 9,
    COL_GF = 10,
    COL_GF_SURFACE = 11,
    COL_SAC_RATE = 12,
    COL_AMB_CONSUMPTION = 13,
    COL_STEP_CONSUMPTION = 14,
    COL_GAS_DENSITY = 15,
    COL_END_WO_O2 = 16,
    COL_END_W_O2 = 17,
    COL_CNS_SINGLE = 18,
    COL_CNS_MULTIPLE = 19,
    COL_OTU = 20,
    DIVE_PLAN_COLUMNS_COUNT = 21
};

enum StopStepColumns {
    STOP_COL_DEPTH = 0,
    STOP_COL_TIME = 1,
    STOP_COL_DELETE = 2,
    STOP_STEP_COLUMNS_COUNT = 3
};

enum GasesTableColumns {
    GAS_COL_O2 = 0,
    GAS_COL_HE = 1,
    GAS_COL_SWITCH_DEPTH = 2,
    GAS_COL_SWITCH_PPO2 = 3,
    GAS_COL_CONSUMPTION = 4,
    GAS_COL_NB_TANKS = 5,
    GAS_COL_TANK_CAPACITY = 6,
    GAS_COL_FILLING_PRESSURE = 7,
    GAS_COL_RESERVE_PRESSURE = 8,
    GAS_COL_END_PRESSURE = 9,
    GASES_TABLE_COLUMNS_COUNT = 10
};

enum SetpointColumns {
    SP_COL_DEPTH = 0,
    SP_COL_SETPOINT = 1,
    SP_COL_DELETE = 2,
    SETPOINT_COLUMNS_COUNT = 3
};


// Dive Plan Window class
class DivePlanWindow : public QMainWindow {
    Q_OBJECT
    
public:
    DivePlanWindow(double depth, double bottomTime, diveMode mode, QWidget *parent = nullptr);
    ~DivePlanWindow() override;

    void setDivePlanningMenu(QMenu* menu);
    void activate();

protected:
    void showEvent(QShowEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;

private:
    MainWindow* m_mainWindow;

    // Window size
    const int preferredWidth = 1250;
    const int preferredHeight = 800;

    // Data members
    std::unique_ptr<DivePlan> m_divePlan;

    // Splitter management
    enum class SplitterDirection {
        HORIZONTAL,
        VERTICAL
    };

    struct SplitterConfig {
        QSplitter* splitter;
        QString name;
        SplitterDirection direction;
        int handleWidth;
        QString handleDecoration;
        QList<int> defaultSizes;
        bool collapsible;
        int minVisibleSize = 1;  // Minimal size when collapsed but still showing handle
        int snapThreshold = 20;  // Threshold for snapping to collapsed state
    };

    QVector<SplitterConfig> m_splitters;
    void setupSplitters();
    void updateWidgetVisibility(QSplitter* splitter, const QList<int>& sizes);
    void configureSplitter(QSplitter* splitter, const QString& name, 
                          SplitterDirection direction, int handleWidth,
                          const QString& handleDecoration, 
                          const QList<int>& defaultSizes, bool collapsible);
    void handleSplitterMovement(QSplitter* splitter, int index);
    void updateSplitterVisibility(QSplitter* splitter);
    
    QWidget* graphicWidget;

    // Menu-related members
    QMenu* m_divePlanningMenu;
    QAction* m_ccModeAction;
    QAction* m_ocModeAction;
    QAction* m_bailoutAction;
    QAction* m_gfBoostedAction;
    
    // Menu methods
    void setupMenu();
    void updateMenuState();

    // UI controls
    QComboBox *modeComboBox;
    QCheckBox *bailoutCheckBox;
    QTableWidget *stopStepsTable;
    QTableWidget* setpointsTable;
    QTableWidget *divePlanTable;
    QTableWidget *gasesTable;
    std::vector<int> m_gasRowToOriginalIndex; // Maps visible row indices to original gas indices

    // Track splitters for standardized behavior
    QSplitter* mainSplitter = nullptr; // Main splitter between left and right panels
    QSplitter* leftPanelSplitter = nullptr; // For setpoints and stop steps
    QSplitter* topWidgetsSplitter = nullptr; // For gas list and placeholder
    std::unique_ptr<QTimer> m_splitterCheckTimer;

    // Collapse/expand table
    QLabel *infoLabel;
    QSplitter *verticalSplitter;
    QPushButton *toggleTableButton;
    bool m_tableDirty = false;
    bool m_isUpdating = false;

    // Store original column widths for proportional resizing
    QVector<int> m_originalColumnWidths;
    int m_totalOriginalWidth = 0;
    bool m_columnsInitialized = false;
    
    // Add gas table related members
    bool m_gasesColumnsInitialized = false;  // Initialize to false!
    std::vector<int> m_gasesColumnWidths;
    int m_totalGasesWidth = 0;  // Initialize to 0!

   // Progress dialog
    std::unique_ptr<QProgressDialog> m_progressDialog;
    void showProgressDialog(const QString& message);

    // UI methods
    void setupUI();
    void setupStopStepsTable();
    void setupDivePlanTable();
    void refreshStopStepsTable();
    void setupSetpointsTable();
    void refreshSetpointsTable();
    void updateSetpointVisibility();
    void refreshDivePlanTable();
    void rebuildDivePlan();
    void refreshDivePlan();
    void highlightWarningCells();
    void setupGasesTable();
    void refreshGasesTable();
    
    void updateGasTablePressures();
    void gasTableCellChanged(int row, int column);

private slots:
    // Registered slots
    void diveModeChanged(int index);
    void bailoutToggled(bool checked);
    void divePlanCellChanged(int row, int column);
    void setpointCellChanged(int row, int column);
    void addSetpoint();
    void deleteSetpoint(int row);
    void stopStepCellChanged(int row, int column);
    void addStopStep();
    void deleteStopStep(int row);
    void onWindowTitleChanged();
    void initializeSplitters();
    void onSplitterMoved(int pos, int index);
    void ccModeActivated();
    
    // Make sure resizeDivePlanTable and resizeGasesTable are declared as slots
    void resizeDivePlanTable();
    void resizeGasesTable();
    
    // PLACEHOLDER methods
    void defineMission();
    void setMaxTime();
    void optimiseDecoGas();

    // Helper methods 
    QString  getPhaseString(Phase phase);
    QString  getStepModeString(stepMode mode);
};

} // namespace DiveComputer

#endif // DIVEPLAN_GUI_HPP