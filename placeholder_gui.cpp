#include "placeholder_gui.hpp"

namespace DiveComputer {

PlaceholderWindow::PlaceholderWindow(QWidget *parent) : QMainWindow(parent) {
    // Set window title
    setWindowTitle("Placeholder Window");
    
    // Create central widget with text label and button
    QWidget *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);
    QVBoxLayout *layout = new QVBoxLayout(centralWidget);
    
    // Add the placeholder label
    placeholderLabel = new QLabel("Placeholder", centralWidget);
    QFont font = placeholderLabel->font();
    font.setPointSize(16);
    placeholderLabel->setFont(font);
    placeholderLabel->setAlignment(Qt::AlignCenter);
    layout->addWidget(placeholderLabel);
    
    // Add some vertical space
    layout->addSpacing(20);
    
    // Add a recalculate button
    recalculateButton = new QPushButton("Recalculate", centralWidget);
    layout->addWidget(recalculateButton, 0, Qt::AlignCenter);
    
    // Connect the button's clicked signal to the recalculate slot
    connect(recalculateButton, &QPushButton::clicked, this, &PlaceholderWindow::recalculate);
    
    // Use the common window sizing and positioning function
    setWindowSizeAndPosition(this, preferredWidth, preferredHeight, WindowPosition::CENTER);
}

void PlaceholderWindow::recalculate() {
    // This is a placeholder function that does nothing right now
    // In a real application, this would perform some calculation and update the UI
    
    // Just for feedback, we'll show a temporary status message
    statusBar()->showMessage("Recalculation complete (placeholder)", 2000);
}

} // namespace DiveComputer