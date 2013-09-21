plasma_install_package(package org.kde.icon)

set(iconplugin_SRCS
  plugin/icon_p.cpp
  plugin/iconplugin.cpp
)

qt4_automoc(${iconplugin_SRCS})

install(FILES plugin/qmldir DESTINATION ${QML_INSTALL_DIR}/org/kde/plasma/icon)

add_library(iconplugin SHARED ${iconplugin_SRCS})

target_link_libraries(iconplugin
  ${QT_QTCORE_LIBRARY}
  ${Qt5Qml_LIBRARIES}
  ${Qt5Quick_LIBRARIES}
  ${KIO_LIBRARIES}
)

install(TARGETS iconplugin DESTINATION ${QML_INSTALL_DIR}/org/kde/plasma/icon)