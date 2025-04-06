QT += core widgets
CONFIG += c++17

RESOURCES += resources.qrc

SOURCES += \
    main.cpp \
    enum.cpp \
    global.cpp \
    constants.cpp \
    parameters.cpp \
    gas.cpp \
    gaslist.cpp \
    buhlmann.cpp \
    compartments.cpp \
    oxygen_toxicity.cpp \
    stop_steps.cpp \
    set_points.cpp \
    dive_step.cpp \
    dive_plan.cpp \
    parameters_gui.cpp \
    gaslist_gui.cpp \
    dive_plan_dialog.cpp \
    dive_plan_gui.cpp \
    dive_plan_gui_stopsteps.cpp \
    dive_plan_gui_plantables.cpp \
    dive_plan_gui_menu.cpp \
    dive_plan_gui_gaslist.cpp \
    dive_plan_gui_setpoints.cpp \
    placeholder_gui.cpp \
    main_gui.cpp

HEADERS += \
    qtheaders.hpp \
    error_handler.hpp \
    global.hpp \
    table_helper.hpp \
    enum.hpp \
    constants.hpp \
    parameters.hpp \
    gas.hpp \
    gaslist.hpp \
    buhlmann.hpp \
    compartments.hpp \
    oxygen_toxicity.hpp \
    stop_steps.hpp \
    set_points.hpp \
    dive_step.hpp \
    dive_plan.hpp \
    parameters_gui.hpp \
    gaslist_gui.hpp \
    dive_plan_dialog.hpp \
    dive_plan_gui.hpp \
    placeholder_gui.hpp \
    ui_utils.hpp \
    main_gui.hpp

macx {
    # Determine SDK path dynamically
    SDK_PATH = $$system(xcrun --show-sdk-path)
    isEmpty(SDK_PATH) {
        error("Could not determine SDK path. Make sure Xcode and Command Line Tools are properly installed.")
    }
    
    message("Using SDK path: $$SDK_PATH")
    
    # Use dynamically determined SDK path
    QMAKE_CXXFLAGS += -isysroot $$SDK_PATH
    QMAKE_LFLAGS += -isysroot $$SDK_PATH
    
    # Set the minimum macOS version to match your system
    QMAKE_MACOSX_DEPLOYMENT_TARGET = 14.0
    
    # Make sure the C++ standard library headers come first in search path
    QMAKE_CXXFLAGS += -stdlib=libc++
    
    # Fix the include path order - libc++ headers must come before system headers
    INCLUDEPATH = $$SDK_PATH/usr/include/c++/v1 $$INCLUDEPATH
    INCLUDEPATH += $$SDK_PATH/usr/include
    
    # Explicitly specify the C++ standard to avoid gnu extensions
    QMAKE_CXXFLAGS += -std=c++17
    
    # Remove any conflicting flags that might be causing issues
    QMAKE_CXXFLAGS -= -std=gnu++1z
}

# Add a post-link step that copies the executable and cleans up intermediate files
macx {
    # Script to:
    # 1. Check if the app exists and copy the executable
    # 2. Clean intermediate files but keep the app bundle and executable
    QMAKE_POST_LINK = \
        test -e $${TARGET}.app/Contents/MacOS/$${TARGET} && \
        cp -f $${TARGET}.app/Contents/MacOS/$${TARGET} . && \
        $(DEL_FILE) $(OBJECTS) && \
        $(DEL_FILE) *~ core *.core moc_*.cpp moc_*.h moc_predefs.h || \
        echo "App not found, skipping copy and cleanup"
}

# Define a full clean target for when you want to remove everything
fullclean.commands = $(DEL_FILE) $(OBJECTS) && \
                   $(DEL_FILE) *~ core *.core && \
                   rm -f $${TARGET} && \
                   rm -rf $${TARGET}.app && \
                   rm -f .qmake.stash

QMAKE_EXTRA_TARGETS += fullclean