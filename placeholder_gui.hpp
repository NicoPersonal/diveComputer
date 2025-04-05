#ifndef PLACEHOLDER_GUI_HPP
#define PLACEHOLDER_GUI_HPP

#include "qtheaders.hpp"
#include "global.hpp"

namespace DiveComputer {

// Class for the placeholder window
class PlaceholderWindow : public QMainWindow {
    Q_OBJECT
    
public:
    PlaceholderWindow(QWidget *parent = nullptr);
    ~PlaceholderWindow() = default;
    
private:
    // Window size
    const int preferredWidth = 300;
    const int preferredHeight = 300;

    QLabel* placeholderLabel;
    QPushButton* recalculateButton;
    
private slots:
    // Placeholder function to be called when Recalculate button is clicked
    void recalculate();
};

} // namespace DiveComputer

#endif // PLACEHOLDER_GUI_HPP