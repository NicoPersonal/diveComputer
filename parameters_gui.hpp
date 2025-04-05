#ifndef PARAMETERS_GUI_HPP
#define PARAMETERS_GUI_HPP

#include "qtheaders.hpp"
#include "parameters.hpp"

namespace DiveComputer {

// Parameter Editor Window class
class ParameterWindow : public QMainWindow {
    Q_OBJECT
    
public:
    ParameterWindow(QWidget *parent = nullptr);
    
private:
    // Window size
    const int preferredWidth = 650;
    const int preferredHeight = 680;

    // Gradient factors
    QDoubleSpinBox *gfLowSpinBox;
    QDoubleSpinBox *gfHighSpinBox;

    // Environment
    QDoubleSpinBox *atmPressureSpinBox;
    QDoubleSpinBox *tempSpinBox;

    // Gas parameters
    QDoubleSpinBox *defaultEndSpinBox;
    QCheckBox      *o2NarcoticCheckBox;

    //  Rate parameters
    QDoubleSpinBox *maxAscentRateSpinBox;
    QDoubleSpinBox *maxDescentRateSpinBox;

    // Cost parameters
    QDoubleSpinBox *sacBottomSpinBox;
    QDoubleSpinBox *sacBailoutSpinBox;
    QDoubleSpinBox *sacDecoSpinBox;
    QDoubleSpinBox *o2CostPerLSpinBox;
    QDoubleSpinBox *heCostPerLSpinBox;

    // O2 levels
    QDoubleSpinBox *bestMixDepthBufferSpinBox;
    QDoubleSpinBox *maxPpO2BottomSpinBox;
    QDoubleSpinBox *maxPpO2DecoSpinBox;
    QDoubleSpinBox *maxPpO2DiluentSpinBox;
    QDoubleSpinBox *warningPpO2LowSpinBox;

    // Warning thresholds
    QDoubleSpinBox *warningCnsMaxSpinBox;
    QDoubleSpinBox *warningOtuMaxSpinBox;
    QDoubleSpinBox *warningGasDensitySpinBox;

    // Stop parameters
    QDoubleSpinBox *depthIncrementSpinBox;
    QDoubleSpinBox *lastStopDepthSpinBox;
    QDoubleSpinBox *timeIncrementDecoSpinBox;

    // No-fly parameters
    QDoubleSpinBox *noFlyPressureSpinBox;
    QDoubleSpinBox *noFlyGfSpinBox;
    QDoubleSpinBox *noFlyTimeIncrementSpinBox;

    // UI Components for each parameter
    QGroupBox* createGradientFactorGroup();
    QGroupBox* createEnvironmentGroup();
    QGroupBox* createGasParametersGroup();
    QGroupBox* createRateParametersGroup();
    QGroupBox* createCostParametersGroup();
    QGroupBox* createO2LevelsGroup();
    QGroupBox* createWarningThresholdsGroup();
    QGroupBox* createStopParametersGroup();
    QGroupBox* createNoFlyParametersGroup();

    void loadParameterValues();

private slots:
    void resetParameters();
    void saveParameters();
};

} // namespace DiveComputer

#endif // PARAMETERS_GUI_HPP