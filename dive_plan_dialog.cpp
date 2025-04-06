#include "dive_plan_dialog.hpp"
#include "error_handler.hpp"

namespace DiveComputer {

DivePlanDialog::DivePlanDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle("Create Dive Plan");
    setupUI();
    
    // Create button disabled by default until valid input
    createButton->setEnabled(false);
    
    // Connect validation signals/slots
    connect(depthEdit, &QLineEdit::textChanged, this, &DivePlanDialog::validateInputs);
    connect(bottomTimeEdit, &QLineEdit::textChanged, this, &DivePlanDialog::validateInputs);
}

void DivePlanDialog::setupUI()
{
    // Main layout
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    
    // Form layout for input fields
    QFormLayout *formLayout = new QFormLayout();
    
    // Depth field with validator
    depthEdit = new QLineEdit(this);
    depthEdit->setPlaceholderText("Enter depth in meters");
    depthEdit->setValidator(new QDoubleValidator(0.1, 300.0, 1, this));
    formLayout->addRow("Depth (m):", depthEdit);
    
    // Bottom time field with validator
    bottomTimeEdit = new QLineEdit(this);
    bottomTimeEdit->setPlaceholderText("Enter bottom time in minutes");
    bottomTimeEdit->setValidator(new QDoubleValidator(0.1, 1000.0, 1, this));
    formLayout->addRow("Bottom Time (min):", bottomTimeEdit);
    
    // Dive mode combo box
    diveModeCombo = new QComboBox(this);
    diveModeCombo->addItem("Open Circuit (OC)", static_cast<int>(diveMode::OC));
    diveModeCombo->addItem("Closed Circuit (CC)", static_cast<int>(diveMode::CC));
    formLayout->addRow("Dive Mode:", diveModeCombo);
    
    mainLayout->addLayout(formLayout);
    
    // Button layout
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    
    createButton = new QPushButton("Create", this);
    connect(createButton, &QPushButton::clicked, this, &QDialog::accept);
    
    cancelButton = new QPushButton("Cancel", this);
    connect(cancelButton, &QPushButton::clicked, this, &QDialog::reject);
    
    buttonLayout->addWidget(createButton);
    buttonLayout->addWidget(cancelButton);
    
    mainLayout->addLayout(buttonLayout);
    
    // Set size policy
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    setMinimumWidth(300);
}

void DivePlanDialog::validateInputs() {
    // Get the depth and bottom time values
    bool depthValid = false;
    bool timeValid = false;
    
    double depth = depthEdit->text().toDouble(&depthValid);
    double time = bottomTimeEdit->text().toDouble(&timeValid);
    
    // Use new validator for depth
    if (depthValid) {
        depthValid = ErrorHandler::validateNumericInput(
            depthEdit->text(), depth, 0.1, 300.0, "Depth", false);
    }
    
    // Use new validator for time
    if (timeValid) {
        timeValid = ErrorHandler::validateNumericInput(
            bottomTimeEdit->text(), time, 0.1, 1000.0, "Bottom Time", false);
    }
    
    // Enable create button if both are valid
    createButton->setEnabled(depthValid && timeValid);
}

double DivePlanDialog::getDepth() const
{
    return depthEdit->text().toDouble();
}

double DivePlanDialog::getBottomTime() const
{
    return bottomTimeEdit->text().toDouble();
}

diveMode DivePlanDialog::getDiveMode() const
{
    return static_cast<diveMode>(diveModeCombo->currentData().toInt());
}

} // namespace DiveComputer