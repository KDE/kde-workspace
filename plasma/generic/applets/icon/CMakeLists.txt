# 
# set(icon_SRCS
#     icon.cpp)
# 
# kde4_add_plugin(plasma_applet_icon ${icon_SRCS})
# target_link_libraries(plasma_applet_icon ${KDE4_PLASMA_LIBS} ${KDE4_KIO_LIBS} ${QT_QTDECLARATIVE_LIBRARY})
# 
# install(TARGETS plasma_applet_icon DESTINATION ${PLUGIN_INSTALL_DIR})
# 
# install(DIRECTORY package/ DESTINATION ${DATA_INSTALL_DIR}/plasma/packages/org.kde.icon)
# 
# install(FILES plasma-applet-icon.desktop DESTINATION ${SERVICES_INSTALL_DIR})

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