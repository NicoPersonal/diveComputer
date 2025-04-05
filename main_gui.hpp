#ifndef MAIN_GUI_HPP
#define MAIN_GUI_HPP

#include "qtheaders.hpp"
#include "parameters_gui.hpp"
#include "placeholder_gui.hpp"
#include "gaslist_gui.hpp"
#include "dive_plan_gui.hpp"

namespace DiveComputer {

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    QMenu* addDivePlanningMenu();
    void removeDivePlanningMenu();

    // Template function to open any type of window
    template <typename WindowType>
    void openWindow(WindowType **windowPtr) {
        // Create the window if it doesn't exist
        if (!(*windowPtr)) {
            *windowPtr = new WindowType(nullptr); // No parent
            (*windowPtr)->setAttribute(Qt::WA_DeleteOnClose);
            
            // Track this window
            childWindows->append(*windowPtr);
            
            // Remove from our list when it's closed
            connect(*windowPtr, &QObject::destroyed, this, &MainWindow::handleWindowDestroyed);
        }
        
        // Show and bring to front
        (*windowPtr)->show();
        (*windowPtr)->activateWindow();
        (*windowPtr)->raise();
    };
    
    void activateWindowWithMenu(DivePlanWindow* window);

protected:
    bool eventFilter(QObject* obj, QEvent* event) override;

private:
    // Window size
    const int preferredWidth = 300;
    const int preferredHeight = 300;

    QLabel *mainLabel;
    QLabel *logoLabel; // New label for logo
    ParameterWindow *parameterWindow = nullptr;
    PlaceholderWindow *placeholderWindow = nullptr;
    GasListWindow *gasListWindow = nullptr;
    QList<QWidget*> *childWindows; // List to track all windows we create

    QMenu* divePlanningMenu = nullptr;
    DivePlanWindow* activeDivePlanWindow = nullptr;

private slots:
    void createDivePlan();
    void quitApplication();
    void openGasListWindow();
    void handleWindowDestroyed();
    void openParameterWindow();
    void openPlaceholderWindow();
    void handleDivePlanWindowDestroyed();
};

} // namespace DiveComputer

#endif // MAIN_GUI_HPP