#ifndef DIVEPLAN_DIALOG_HPP
#define DIVEPLAN_DIALOG_HPP

#include "qtheaders.hpp"
#include "enum.hpp"
#include "error_handler.hpp"

namespace DiveComputer {

// Dialog to create a new dive plan
class DivePlanDialog : public QDialog {
    Q_OBJECT
    
public:
    DivePlanDialog(QWidget *parent = nullptr);
    
    double getDepth() const;
    double getBottomTime() const;
    diveMode getDiveMode() const;
    
private:
    void setupUI();
    void validateInputs();
    
    QLineEdit *depthEdit;
    QLineEdit *bottomTimeEdit;
    QComboBox *diveModeCombo;
    QPushButton *createButton;
    QPushButton *cancelButton;
};

} // namespace DiveComputer

#endif // DIVEPLAN_DIALOG_HPP