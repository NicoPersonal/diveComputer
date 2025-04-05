#include "parameters_gui.hpp"

namespace DiveComputer {

// External global variable defined in parameters.cpp
extern Parameters g_parameters;

ParameterWindow::ParameterWindow(QWidget *parent) : QMainWindow(parent) {
    setWindowTitle("Dive Computer Parameters");
    
    // Create main widget
    QWidget *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);
    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->setSpacing(10);
    
    // Create a scroll area for the parameter groups
    QScrollArea *scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);
    
    QWidget *scrollContentWidget = new QWidget(scrollArea);
    scrollArea->setWidget(scrollContentWidget);
    
    // Create layout for the scroll content widget
    QVBoxLayout *scrollLayout = new QVBoxLayout(scrollContentWidget);
    scrollLayout->setSpacing(10);
    
    // Create top layout for two columns
    QHBoxLayout *columnsLayout = new QHBoxLayout();
    columnsLayout->setSpacing(15);
    
    // Left column layout with button container
    QVBoxLayout *leftColumnLayout = new QVBoxLayout();
    leftColumnLayout->setSpacing(10);
    
    // Right column layout
    QVBoxLayout *rightColumnLayout = new QVBoxLayout();
    rightColumnLayout->setSpacing(10);
    
    // Set equal size for the columns
    columnsLayout->addLayout(leftColumnLayout, 1);
    columnsLayout->addLayout(rightColumnLayout, 1);
    
    // Add groups to columns
    leftColumnLayout->addWidget(createGradientFactorGroup());
    leftColumnLayout->addWidget(createEnvironmentGroup());
    leftColumnLayout->addWidget(createGasParametersGroup());
    leftColumnLayout->addWidget(createRateParametersGroup());
    leftColumnLayout->addWidget(createO2LevelsGroup());
    
    // Create buttons layout to be placed under all groups
    QWidget *buttonWidget = new QWidget(this);
    QHBoxLayout *buttonLayout = new QHBoxLayout(buttonWidget);
    buttonLayout->setContentsMargins(5, 5, 5, 5);
    
    // Create the buttons
    QPushButton *saveButton = new QPushButton("Save", this);
    QPushButton *defaultButton = new QPushButton("Default", this);
    QPushButton *cancelButton = new QPushButton("Cancel", this);
    
    // Center the buttons across the window width
    buttonLayout->addStretch(1);
    buttonLayout->addWidget(saveButton);
    buttonLayout->addWidget(defaultButton);
    buttonLayout->addWidget(cancelButton);
    buttonLayout->addStretch(1);
    
    // Add groups to right column
    rightColumnLayout->addWidget(createCostParametersGroup());
    rightColumnLayout->addWidget(createWarningThresholdsGroup());
    rightColumnLayout->addWidget(createStopParametersGroup());
    rightColumnLayout->addWidget(createNoFlyParametersGroup());
    
    // Add stretches to push content to the top
    leftColumnLayout->addStretch(1);
    rightColumnLayout->addStretch(1);
    
    // Add columns layout to scroll layout
    scrollLayout->addLayout(columnsLayout);
    
    // Add button widget to main layout (not to left column layout)
    // This ensures the buttons are centered across the entire window
    scrollLayout->addWidget(buttonWidget);
    
    // Add scroll area to main layout
    mainLayout->addWidget(scrollArea);
    
    // Connect buttons
    connect(saveButton, &QPushButton::clicked, this, &ParameterWindow::saveParameters);
    connect(defaultButton, &QPushButton::clicked, this, &ParameterWindow::resetParameters);
    connect(cancelButton, &QPushButton::clicked, this, &QWidget::close);
    
    // Load initial values
    loadParameterValues();
    
    // Use the common window sizing and positioning function
    setWindowSizeAndPosition(this, preferredWidth, preferredHeight, WindowPosition::TOP_LEFT);
}
     
QGroupBox* ParameterWindow::createGradientFactorGroup() {
    QGroupBox *gfGroup = new QGroupBox("Gradient Factors", this);
    QGridLayout *gridLayout = new QGridLayout(gfGroup);
    
    // Create labels and position them in the left side, right-justified
    QLabel *gfLowLabel = new QLabel("GF Low:", this);
    QLabel *gfHighLabel = new QLabel("GF High:", this);
    
    gfLowLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    gfHighLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    
    // Create spinboxes and position them in the right side, right-justified
    gfLowSpinBox = new QDoubleSpinBox(this);
    gfLowSpinBox->setRange(0.0, 100.0);
    gfLowSpinBox->setSingleStep(5.0);
    gfLowSpinBox->setDecimals(0);
    gfLowSpinBox->setSuffix("%");
    gfLowSpinBox->setFixedWidth(100);
    gfLowSpinBox->setAlignment(Qt::AlignRight);
    
    gfHighSpinBox = new QDoubleSpinBox(this);
    gfHighSpinBox->setRange(0.0, 100.0);
    gfHighSpinBox->setSingleStep(5.0);
    gfHighSpinBox->setDecimals(0);
    gfHighSpinBox->setSuffix("%");
    gfHighSpinBox->setFixedWidth(100);
    gfHighSpinBox->setAlignment(Qt::AlignRight);
    
    // Add widgets to grid layout
    gridLayout->addWidget(gfLowLabel, 0, 0);
    gridLayout->addWidget(gfLowSpinBox, 0, 1);
    gridLayout->addWidget(gfHighLabel, 1, 0);
    gridLayout->addWidget(gfHighSpinBox, 1, 1);
    
    // Configure column stretching to maintain alignments during resize
    gridLayout->setColumnStretch(0, 1);  // Left column (labels) will expand
    gridLayout->setColumnStretch(1, 1);  // Right column (fields) will expand
    
    return gfGroup;
}
    
QGroupBox* ParameterWindow::createEnvironmentGroup() {
    QGroupBox *envGroup = new QGroupBox("Environment", this);
    QGridLayout *gridLayout = new QGridLayout(envGroup);
    
    // Create labels with right alignment
    QLabel *atmPressureLabel = new QLabel("Atm. Pressure:", this);
    QLabel *tempLabel = new QLabel("Temperature:", this);
    
    atmPressureLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    tempLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    
    // Create spinboxes with right alignment
    atmPressureSpinBox = new QDoubleSpinBox(this);
    atmPressureSpinBox->setRange(0.0, 2.0);
    atmPressureSpinBox->setDecimals(5);
    atmPressureSpinBox->setSingleStep(0.10);
    atmPressureSpinBox->setSuffix(" bar");
    atmPressureSpinBox->setFixedWidth(100);
    atmPressureSpinBox->setAlignment(Qt::AlignRight);
    
    tempSpinBox = new QDoubleSpinBox(this);
    tempSpinBox->setRange(0.0, 40.0);
    tempSpinBox->setSingleStep(5);
    tempSpinBox->setSuffix(" °C");
    tempSpinBox->setFixedWidth(100);
    tempSpinBox->setAlignment(Qt::AlignRight);
    
    // Add widgets to grid layout
    gridLayout->addWidget(atmPressureLabel, 0, 0);
    gridLayout->addWidget(atmPressureSpinBox, 0, 1);
    gridLayout->addWidget(tempLabel, 1, 0);
    gridLayout->addWidget(tempSpinBox, 1, 1);
    
    // Configure column stretching
    gridLayout->setColumnStretch(0, 1);
    gridLayout->setColumnStretch(1, 1);
    
    return envGroup;
}
    
QGroupBox* ParameterWindow::createGasParametersGroup() {
    QGroupBox *gasGroup = new QGroupBox("Gas Parameters", this);
    QGridLayout *gridLayout = new QGridLayout(gasGroup);
    
    // Create labels with right alignment
    QLabel *defaultEndLabel = new QLabel("Default END:", this);
    QLabel *o2NarcoticLabel = new QLabel("O2 Narcotic:", this);
    
    defaultEndLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    o2NarcoticLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    
    // Create spinbox with right alignment
    defaultEndSpinBox = new QDoubleSpinBox(this);
    defaultEndSpinBox->setRange(0.0, 100.0);
    defaultEndSpinBox->setSingleStep(1.0);
    defaultEndSpinBox->setDecimals(0);
    defaultEndSpinBox->setSuffix(" m");
    defaultEndSpinBox->setFixedWidth(100);
    defaultEndSpinBox->setAlignment(Qt::AlignRight);
    
    // Create checkbox with left alignment (in the right column)
    o2NarcoticCheckBox = new QCheckBox(this);
    QWidget* checkboxContainer = new QWidget();
    QHBoxLayout* checkboxLayout = new QHBoxLayout(checkboxContainer);
    checkboxLayout->setContentsMargins(0, 0, 0, 0);
    checkboxLayout->setAlignment(Qt::AlignLeft);
    checkboxLayout->addWidget(o2NarcoticCheckBox);
    
    // Add widgets to grid layout
    gridLayout->addWidget(defaultEndLabel, 0, 0);
    gridLayout->addWidget(defaultEndSpinBox, 0, 1);
    gridLayout->addWidget(o2NarcoticLabel, 1, 0);
    gridLayout->addWidget(checkboxContainer, 1, 1);
    
    // Configure column stretching
    gridLayout->setColumnStretch(0, 1);
    gridLayout->setColumnStretch(1, 1);
    
    return gasGroup;
}
    
QGroupBox* ParameterWindow::createRateParametersGroup() {
    QGroupBox *rateGroup = new QGroupBox("Rate Parameters", this);
    QGridLayout *gridLayout = new QGridLayout(rateGroup);
    
    // Create labels with right alignment
    QLabel *maxAscentRateLabel = new QLabel("Max Ascent Rate:", this);
    QLabel *maxDescentRateLabel = new QLabel("Max Descent Rate:", this);
    
    maxAscentRateLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    maxDescentRateLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    
    // Create spinboxes with right alignment
    maxAscentRateSpinBox = new QDoubleSpinBox(this);
    maxAscentRateSpinBox->setRange(1.0, 30.0);
    maxAscentRateSpinBox->setSingleStep(1.0);
    maxAscentRateSpinBox->setDecimals(0);
    maxAscentRateSpinBox->setSuffix(" m/min");
    maxAscentRateSpinBox->setFixedWidth(100);
    maxAscentRateSpinBox->setAlignment(Qt::AlignRight);
    
    maxDescentRateSpinBox = new QDoubleSpinBox(this);
    maxDescentRateSpinBox->setRange(1.0, 50.0);
    maxDescentRateSpinBox->setSingleStep(1.0);
    maxDescentRateSpinBox->setDecimals(0);
    maxDescentRateSpinBox->setSuffix(" m/min");
    maxDescentRateSpinBox->setFixedWidth(100);
    maxDescentRateSpinBox->setAlignment(Qt::AlignRight);
    
    // Add widgets to grid layout
    gridLayout->addWidget(maxAscentRateLabel, 0, 0);
    gridLayout->addWidget(maxAscentRateSpinBox, 0, 1);
    gridLayout->addWidget(maxDescentRateLabel, 1, 0);
    gridLayout->addWidget(maxDescentRateSpinBox, 1, 1);
    
    // Configure column stretching
    gridLayout->setColumnStretch(0, 1);
    gridLayout->setColumnStretch(1, 1);
    
    return rateGroup;
}
    
QGroupBox* ParameterWindow::createCostParametersGroup() {
    QGroupBox *costGroup = new QGroupBox("Gas Consumption and Cost", this);
    QGridLayout *gridLayout = new QGridLayout(costGroup);
    
    // Create labels with right alignment
    QLabel *sacBottomLabel = new QLabel("SAC Bottom:", this);
    QLabel *sacBailoutLabel = new QLabel("SAC Bailout:", this);
    QLabel *sacDecoLabel = new QLabel("SAC Deco:", this);
    QLabel *o2CostPerLLabel = new QLabel("O2 Cost Per Liter:", this);
    QLabel *heCostPerLLabel = new QLabel("He Cost Per Liter:", this);
    
    sacBottomLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    sacBailoutLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    sacDecoLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    o2CostPerLLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    heCostPerLLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    
    // Create spinboxes with right alignment
    sacBottomSpinBox = new QDoubleSpinBox(this);
    sacBottomSpinBox->setRange(5.0, 50.0);
    sacBottomSpinBox->setSingleStep(1.0);
    sacBottomSpinBox->setDecimals(0);
    sacBottomSpinBox->setSuffix(" L/min");
    sacBottomSpinBox->setFixedWidth(100);
    sacBottomSpinBox->setAlignment(Qt::AlignRight);
    
    sacBailoutSpinBox = new QDoubleSpinBox(this);
    sacBailoutSpinBox->setRange(5.0, 50.0);
    sacBailoutSpinBox->setSingleStep(1.0);
    sacBailoutSpinBox->setDecimals(0);
    sacBailoutSpinBox->setSuffix(" L/min");
    sacBailoutSpinBox->setFixedWidth(100);
    sacBailoutSpinBox->setAlignment(Qt::AlignRight);
    
    sacDecoSpinBox = new QDoubleSpinBox(this);
    sacDecoSpinBox->setRange(5.0, 50.0);
    sacDecoSpinBox->setSingleStep(1.0);
    sacDecoSpinBox->setDecimals(0);
    sacDecoSpinBox->setSuffix(" L/min");
    sacDecoSpinBox->setFixedWidth(100);
    sacDecoSpinBox->setAlignment(Qt::AlignRight);
    
    o2CostPerLSpinBox = new QDoubleSpinBox(this);
    o2CostPerLSpinBox->setRange(0.0, 1.0);
    o2CostPerLSpinBox->setDecimals(3);
    o2CostPerLSpinBox->setSingleStep(0.01);
    o2CostPerLSpinBox->setPrefix("$");
    o2CostPerLSpinBox->setFixedWidth(100);
    o2CostPerLSpinBox->setAlignment(Qt::AlignRight);
    
    heCostPerLSpinBox = new QDoubleSpinBox(this);
    heCostPerLSpinBox->setRange(0.0, 1.0);
    heCostPerLSpinBox->setDecimals(3);
    heCostPerLSpinBox->setSingleStep(0.01);
    heCostPerLSpinBox->setPrefix("$");
    heCostPerLSpinBox->setFixedWidth(100);
    heCostPerLSpinBox->setAlignment(Qt::AlignRight);
    
    // Add widgets to grid layout
    gridLayout->addWidget(sacBottomLabel, 0, 0);
    gridLayout->addWidget(sacBottomSpinBox, 0, 1);
    gridLayout->addWidget(sacBailoutLabel, 1, 0);
    gridLayout->addWidget(sacBailoutSpinBox, 1, 1);
    gridLayout->addWidget(sacDecoLabel, 2, 0);
    gridLayout->addWidget(sacDecoSpinBox, 2, 1);
    gridLayout->addWidget(o2CostPerLLabel, 3, 0);
    gridLayout->addWidget(o2CostPerLSpinBox, 3, 1);
    gridLayout->addWidget(heCostPerLLabel, 4, 0);
    gridLayout->addWidget(heCostPerLSpinBox, 4, 1);
    
    // Configure column stretching
    gridLayout->setColumnStretch(0, 1);
    gridLayout->setColumnStretch(1, 1);
    
    return costGroup;
}

QGroupBox* ParameterWindow::createO2LevelsGroup() {
    QGroupBox *o2Group = new QGroupBox("O2 Levels", this);
    QGridLayout *gridLayout = new QGridLayout(o2Group);
    
    // Create labels with right alignment
    QLabel *bestMixDepthBufferLabel = new QLabel("Best Mix Depth Buffer:", this);
    QLabel *maxPpO2BottomLabel = new QLabel("Max ppO2 Active:", this);
    QLabel *maxPpO2DecoLabel = new QLabel("Max ppO2 Deco:", this);
    QLabel *maxPpO2DiluentLabel = new QLabel("Max ppO2 Diluent:", this);
    QLabel *warningPpO2LowLabel = new QLabel("Min ppO2 (Hypoxia):", this);
    
    bestMixDepthBufferLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    maxPpO2BottomLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    maxPpO2DecoLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    maxPpO2DiluentLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    warningPpO2LowLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    
    // Create spinboxes with right alignment
    bestMixDepthBufferSpinBox = new QDoubleSpinBox(this);
    bestMixDepthBufferSpinBox->setRange(0.0, 20.0);
    bestMixDepthBufferSpinBox->setSingleStep(5);
    bestMixDepthBufferSpinBox->setDecimals(0);
    bestMixDepthBufferSpinBox->setSuffix(" m");
    bestMixDepthBufferSpinBox->setFixedWidth(100);
    bestMixDepthBufferSpinBox->setAlignment(Qt::AlignRight);
    
    maxPpO2BottomSpinBox = new QDoubleSpinBox(this);
    maxPpO2BottomSpinBox->setRange(0.7, 1.8);
    maxPpO2BottomSpinBox->setDecimals(2);
    maxPpO2BottomSpinBox->setSingleStep(0.10);
    maxPpO2BottomSpinBox->setSuffix(" bar");
    maxPpO2BottomSpinBox->setFixedWidth(100);
    maxPpO2BottomSpinBox->setAlignment(Qt::AlignRight);
    
    maxPpO2DecoSpinBox = new QDoubleSpinBox(this);
    maxPpO2DecoSpinBox->setRange(0.7, 1.8);
    maxPpO2DecoSpinBox->setDecimals(2);
    maxPpO2DecoSpinBox->setSingleStep(0.10);
    maxPpO2DecoSpinBox->setSuffix(" bar");
    maxPpO2DecoSpinBox->setFixedWidth(100);
    maxPpO2DecoSpinBox->setAlignment(Qt::AlignRight);
    
    maxPpO2DiluentSpinBox = new QDoubleSpinBox(this);
    maxPpO2DiluentSpinBox->setRange(0.7, 1.8);
    maxPpO2DiluentSpinBox->setDecimals(2);
    maxPpO2DiluentSpinBox->setSingleStep(0.10);
    maxPpO2DiluentSpinBox->setSuffix(" bar");
    maxPpO2DiluentSpinBox->setFixedWidth(100);
    maxPpO2DiluentSpinBox->setAlignment(Qt::AlignRight);
    
    warningPpO2LowSpinBox = new QDoubleSpinBox(this);
    warningPpO2LowSpinBox->setRange(0.1, 0.21);
    warningPpO2LowSpinBox->setDecimals(2);
    warningPpO2LowSpinBox->setSingleStep(0.01);
    warningPpO2LowSpinBox->setSuffix(" bar");
    warningPpO2LowSpinBox->setFixedWidth(100);
    warningPpO2LowSpinBox->setAlignment(Qt::AlignRight);
    
    // Add widgets to grid layout
    gridLayout->addWidget(bestMixDepthBufferLabel, 0, 0);
    gridLayout->addWidget(bestMixDepthBufferSpinBox, 0, 1);
    gridLayout->addWidget(maxPpO2BottomLabel, 1, 0);
    gridLayout->addWidget(maxPpO2BottomSpinBox, 1, 1);
    gridLayout->addWidget(maxPpO2DecoLabel, 2, 0);
    gridLayout->addWidget(maxPpO2DecoSpinBox, 2, 1);
    gridLayout->addWidget(maxPpO2DiluentLabel, 3, 0);
    gridLayout->addWidget(maxPpO2DiluentSpinBox, 3, 1);
    gridLayout->addWidget(warningPpO2LowLabel, 4, 0);
    gridLayout->addWidget(warningPpO2LowSpinBox, 4, 1);
    
    // Configure column stretching
    gridLayout->setColumnStretch(0, 1);
    gridLayout->setColumnStretch(1, 1);
    
    return o2Group;
}
    
QGroupBox* ParameterWindow::createWarningThresholdsGroup() {
    QGroupBox *warningGroup = new QGroupBox("Warning Thresholds", this);
    QGridLayout *gridLayout = new QGridLayout(warningGroup);
    
    // Create labels with right alignment
    QLabel *warningCnsMaxLabel = new QLabel("Warning CNS Max:", this);
    QLabel *warningOtuMaxLabel = new QLabel("Warning OTU Max:", this);
    QLabel *warningGasDensityLabel = new QLabel("Warning Gas Density:", this);
    
    warningCnsMaxLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    warningOtuMaxLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    warningGasDensityLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    
    // Create spinboxes with right alignment
    warningCnsMaxSpinBox = new QDoubleSpinBox(this);
    warningCnsMaxSpinBox->setRange(50.0, 100.0);
    warningCnsMaxSpinBox->setSingleStep(5.0);
    warningCnsMaxSpinBox->setDecimals(0);
    warningCnsMaxSpinBox->setSuffix("%");
    warningCnsMaxSpinBox->setFixedWidth(100);
    warningCnsMaxSpinBox->setAlignment(Qt::AlignRight);
    
    warningOtuMaxSpinBox = new QDoubleSpinBox(this);
    warningOtuMaxSpinBox->setRange(100.0, 500.0);
    warningOtuMaxSpinBox->setSingleStep(50.0);
    warningOtuMaxSpinBox->setDecimals(0);
    warningOtuMaxSpinBox->setFixedWidth(100);
    warningOtuMaxSpinBox->setAlignment(Qt::AlignRight);
    
    warningGasDensitySpinBox = new QDoubleSpinBox(this);
    warningGasDensitySpinBox->setRange(3.0, 10.0);
    warningGasDensitySpinBox->setDecimals(1);
    warningGasDensitySpinBox->setSingleStep(1.0);
    warningGasDensitySpinBox->setSuffix(" g/L");
    warningGasDensitySpinBox->setFixedWidth(100);
    warningGasDensitySpinBox->setAlignment(Qt::AlignRight);
    
    // Add widgets to grid layout
    gridLayout->addWidget(warningCnsMaxLabel, 0, 0);
    gridLayout->addWidget(warningCnsMaxSpinBox, 0, 1);
    gridLayout->addWidget(warningOtuMaxLabel, 1, 0);
    gridLayout->addWidget(warningOtuMaxSpinBox, 1, 1);
    gridLayout->addWidget(warningGasDensityLabel, 2, 0);
    gridLayout->addWidget(warningGasDensitySpinBox, 2, 1);
    
    // Configure column stretching
    gridLayout->setColumnStretch(0, 1);
    gridLayout->setColumnStretch(1, 1);
    
    return warningGroup;
}
    
QGroupBox* ParameterWindow::createStopParametersGroup() {
    QGroupBox *stopGroup = new QGroupBox("Stop Parameters", this);
    QGridLayout *gridLayout = new QGridLayout(stopGroup);
    
    // Create labels with right alignment
    QLabel *depthIncrementLabel = new QLabel("Depth Increment:", this);
    QLabel *lastStopDepthLabel = new QLabel("Last Stop Depth:", this);
    QLabel *timeIncrementDecoLabel = new QLabel("Time Increment Deco:", this);
    
    depthIncrementLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    lastStopDepthLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    timeIncrementDecoLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    
    // Create spinboxes with right alignment
    depthIncrementSpinBox = new QDoubleSpinBox(this);
    depthIncrementSpinBox->setRange(1.0, 10.0);
    depthIncrementSpinBox->setSingleStep(0.5);
    depthIncrementSpinBox->setDecimals(0);
    depthIncrementSpinBox->setSuffix(" m");
    depthIncrementSpinBox->setFixedWidth(100);
    depthIncrementSpinBox->setAlignment(Qt::AlignRight);
    
    lastStopDepthSpinBox = new QDoubleSpinBox(this);
    lastStopDepthSpinBox->setRange(1.0, 10.0);
    lastStopDepthSpinBox->setSingleStep(1.0);
    lastStopDepthSpinBox->setDecimals(0);
    lastStopDepthSpinBox->setSuffix(" m");
    lastStopDepthSpinBox->setFixedWidth(100);
    lastStopDepthSpinBox->setAlignment(Qt::AlignRight);
    
    timeIncrementDecoSpinBox = new QDoubleSpinBox(this);
    timeIncrementDecoSpinBox->setRange(0.0, 5.0);
    timeIncrementDecoSpinBox->setSingleStep(0.5);
    timeIncrementDecoSpinBox->setDecimals(2);
    timeIncrementDecoSpinBox->setSuffix(" min");
    timeIncrementDecoSpinBox->setFixedWidth(100);
    timeIncrementDecoSpinBox->setAlignment(Qt::AlignRight);
    
    // Add widgets to grid layout
    gridLayout->addWidget(depthIncrementLabel, 0, 0);
    gridLayout->addWidget(depthIncrementSpinBox, 0, 1);
    gridLayout->addWidget(lastStopDepthLabel, 1, 0);
    gridLayout->addWidget(lastStopDepthSpinBox, 1, 1);
    gridLayout->addWidget(timeIncrementDecoLabel, 2, 0);
    gridLayout->addWidget(timeIncrementDecoSpinBox, 2, 1);
    
    // Configure column stretching
    gridLayout->setColumnStretch(0, 1);
    gridLayout->setColumnStretch(1, 1);
    
    return stopGroup;
}
    
QGroupBox* ParameterWindow::createNoFlyParametersGroup() {
    QGroupBox *noFlyGroup = new QGroupBox("No-Fly Parameters", this);
    QGridLayout *gridLayout = new QGridLayout(noFlyGroup);
    
    // Create labels with right alignment
    QLabel *noFlyPressureLabel = new QLabel("No-Fly Pressure:", this);
    QLabel *noFlyGfLabel = new QLabel("No-Fly GF:", this);
    QLabel *noFlyTimeIncrementLabel = new QLabel("No-Fly Time Increment:", this);
    
    noFlyPressureLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    noFlyGfLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    noFlyTimeIncrementLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    
    // Create spinboxes with right alignment
    noFlyPressureSpinBox = new QDoubleSpinBox(this);
    noFlyPressureSpinBox->setRange(0.5, 2.0);
    noFlyPressureSpinBox->setDecimals(2);
    noFlyPressureSpinBox->setSingleStep(0.1);
    noFlyPressureSpinBox->setSuffix(" bar");
    noFlyPressureSpinBox->setFixedWidth(100);
    noFlyPressureSpinBox->setAlignment(Qt::AlignRight);
    
    noFlyGfSpinBox = new QDoubleSpinBox(this);
    noFlyGfSpinBox->setRange(0.0, 100.0);
    noFlyGfSpinBox->setSingleStep(5.0);
    noFlyGfSpinBox->setDecimals(0);
    noFlyGfSpinBox->setSuffix("%");
    noFlyGfSpinBox->setFixedWidth(100);
    noFlyGfSpinBox->setAlignment(Qt::AlignRight);
    
    noFlyTimeIncrementSpinBox = new QDoubleSpinBox(this);
    noFlyTimeIncrementSpinBox->setRange(5.0, 60.0);
    noFlyTimeIncrementSpinBox->setSingleStep(5.0);
    noFlyTimeIncrementSpinBox->setDecimals(0);
    noFlyTimeIncrementSpinBox->setSuffix(" min");
    noFlyTimeIncrementSpinBox->setFixedWidth(100);
    noFlyTimeIncrementSpinBox->setAlignment(Qt::AlignRight);
    
    // Add widgets to grid layout
    gridLayout->addWidget(noFlyPressureLabel, 0, 0);
    gridLayout->addWidget(noFlyPressureSpinBox, 0, 1);
    gridLayout->addWidget(noFlyGfLabel, 1, 0);
    gridLayout->addWidget(noFlyGfSpinBox, 1, 1);
    gridLayout->addWidget(noFlyTimeIncrementLabel, 2, 0);
    gridLayout->addWidget(noFlyTimeIncrementSpinBox, 2, 1);
    
    // Configure column stretching
    gridLayout->setColumnStretch(0, 1);
    gridLayout->setColumnStretch(1, 1);
    
    return noFlyGroup;
}
    
void ParameterWindow::loadParameterValues() {
    // Load values from g_parameters
    gfLowSpinBox->setValue(g_parameters.m_gf[0]);
    gfHighSpinBox->setValue(g_parameters.m_gf[1]);
    atmPressureSpinBox->setValue(g_parameters.m_atmPressure);
    tempSpinBox->setValue(g_parameters.m_tempMin); // Renamed variable
    defaultEndSpinBox->setValue(g_parameters.m_defaultEnd);
    o2NarcoticCheckBox->setChecked(g_parameters.m_defaultO2Narcotic);
    maxAscentRateSpinBox->setValue(g_parameters.m_maxAscentRate);
    maxDescentRateSpinBox->setValue(g_parameters.m_maxDescentRate);
    sacBottomSpinBox->setValue(g_parameters.m_sacBottom);
    sacBailoutSpinBox->setValue(g_parameters.m_sacBailout);
    sacDecoSpinBox->setValue(g_parameters.m_sacDeco);
    o2CostPerLSpinBox->setValue(g_parameters.m_o2CostPerL);
    heCostPerLSpinBox->setValue(g_parameters.m_heCostPerL);
    bestMixDepthBufferSpinBox->setValue(g_parameters.m_bestMixDepthBuffer);
    maxPpO2BottomSpinBox->setValue(g_parameters.m_PpO2Active);
    maxPpO2DecoSpinBox->setValue(g_parameters.m_PpO2Deco);
    maxPpO2DiluentSpinBox->setValue(g_parameters.m_maxPpO2Diluent);
    warningPpO2LowSpinBox->setValue(g_parameters.m_warningPpO2Low);
    warningCnsMaxSpinBox->setValue(g_parameters.m_warningCnsMax);
    warningOtuMaxSpinBox->setValue(g_parameters.m_warningOtuMax);
    warningGasDensitySpinBox->setValue(g_parameters.m_warningGasDensity);
    depthIncrementSpinBox->setValue(g_parameters.m_depthIncrement);
    lastStopDepthSpinBox->setValue(g_parameters.m_lastStopDepth);
    timeIncrementDecoSpinBox->setValue(g_parameters.m_timeIncrementDeco);
    noFlyPressureSpinBox->setValue(g_parameters.m_noFlyPressure);
    noFlyGfSpinBox->setValue(g_parameters.m_noFlyGf);
    noFlyTimeIncrementSpinBox->setValue(g_parameters.m_noFlyTimeIncrement);
}

void ParameterWindow::resetParameters() {
    g_parameters.setToDefault();
    loadParameterValues();
}

void ParameterWindow::saveParameters() {
    // Save values back to g_parameters
    g_parameters.m_gf[0] = gfLowSpinBox->value();
    g_parameters.m_gf[1] = gfHighSpinBox->value();
    g_parameters.m_atmPressure = atmPressureSpinBox->value();
    g_parameters.m_tempMin = tempSpinBox->value();
    g_parameters.m_defaultEnd = defaultEndSpinBox->value();
    g_parameters.m_defaultO2Narcotic = o2NarcoticCheckBox->isChecked();
    g_parameters.m_maxAscentRate = maxAscentRateSpinBox->value();
    g_parameters.m_maxDescentRate = maxDescentRateSpinBox->value();
    g_parameters.m_sacBottom = sacBottomSpinBox->value();
    g_parameters.m_sacBailout = sacBailoutSpinBox->value();
    g_parameters.m_sacDeco = sacDecoSpinBox->value();
    g_parameters.m_o2CostPerL = o2CostPerLSpinBox->value();
    g_parameters.m_heCostPerL = heCostPerLSpinBox->value();
    g_parameters.m_bestMixDepthBuffer = bestMixDepthBufferSpinBox->value();
    g_parameters.m_PpO2Active = maxPpO2BottomSpinBox->value();
    g_parameters.m_PpO2Deco = maxPpO2DecoSpinBox->value();
    g_parameters.m_maxPpO2Diluent = maxPpO2DiluentSpinBox->value();
    g_parameters.m_warningPpO2Low = warningPpO2LowSpinBox->value();
    g_parameters.m_warningCnsMax = warningCnsMaxSpinBox->value();
    g_parameters.m_warningOtuMax = warningOtuMaxSpinBox->value();
    g_parameters.m_warningGasDensity = warningGasDensitySpinBox->value();
    g_parameters.m_depthIncrement = depthIncrementSpinBox->value();
    g_parameters.m_lastStopDepth = lastStopDepthSpinBox->value();
    g_parameters.m_timeIncrementDeco = timeIncrementDecoSpinBox->value();
    g_parameters.m_noFlyPressure = noFlyPressureSpinBox->value();
    g_parameters.m_noFlyGf = noFlyGfSpinBox->value();
    g_parameters.m_noFlyTimeIncrement = noFlyTimeIncrementSpinBox->value();
    
    g_parameters.saveParametersToFile();
    
    // Close the window
    close();
}

} // namespace DiveComputer