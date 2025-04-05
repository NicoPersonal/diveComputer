#ifndef UI_UTILS_HPP
#define UI_UTILS_HPP

#include "qtheaders.hpp"
#include <memory>

namespace DiveComputer {

// Utility function to create a delete button widget
// Takes a callback function that will be called when the delete button is clicked
template<typename Func>
std::unique_ptr<QWidget> createDeleteButtonWidget(Func callback) {
    auto widget = std::make_unique<QWidget>();
    QHBoxLayout* layout = new QHBoxLayout(widget.get());
    layout->setAlignment(Qt::AlignCenter);
    layout->setContentsMargins(2, 2, 2, 2);
    
    QPushButton* deleteButton = new QPushButton("×", widget.get());
    deleteButton->setFixedSize(20, 20);
    deleteButton->setToolTip("Delete");
    
    QObject::connect(deleteButton, &QPushButton::clicked, callback);
    
    layout->addWidget(deleteButton);
    return widget;
}

} // namespace DiveComputer

#endif // UI_UTILS_HPP