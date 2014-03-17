project(windowedwidgetsrunner)

include_directories( ${QT_INCLUDES} ${KDE4_INCLUDES} )

set(krunner_windowedwidgets_SRCS
    windowedwidgetsrunner.cpp
)

add_library(krunner_windowedwidgets MODULE ${krunner_windowedwidgets_SRCS})
target_link_libraries(krunner_windowedwidgets KF5::Plasma)

install(TARGETS krunner_windowedwidgets DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES plasma-runner-windowedwidgets.desktop DESTINATION ${SERVICES_INSTALL_DIR})
