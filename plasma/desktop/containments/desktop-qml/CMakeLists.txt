project(plasma-containment-desktopqml)

#include (KDE4Defaults)

# include_directories(
#   ${CMAKE_SOURCE_DIR}
#   ${CMAKE_CURRENT_SOURCE_DIR}
#   ${CMAKE_CURRENT_BINARY_DIR}
#   )
#
# set(desktopqml_SRCS
#     desktop.cpp
#   )
#
# #add_definitions(${QT_DEFINITIONS} ${KDE4_DEFINITIONS})
#
# kde4_add_plugin(plasma_containment_desktopqml ${desktopqml_SRCS})
# target_link_libraries(plasma_containment_desktopqml ${KDE4_PLASMA_LIBS} ${QT_QTDECLARATIVE_LIBRARY}  ${KDE4_KIO_LIBS})
#
# install(TARGETS plasma_containment_desktopqml DESTINATION ${PLUGIN_INSTALL_DIR})
# install(FILES plasma-containment-desktopqml.desktop DESTINATION ${SERVICES_INSTALL_DIR})


# install(DIRECTORY package/ DESTINATION ${DATA_INSTALL_DIR}/plasma/packages/org.kde.plasma.desktopcontainment)
# install(FILES package/metadata.desktop DESTINATION ${SERVICES_INSTALL_DIR} RENAME plasma-package-org.kde.plasma.desktopcontainment.desktop)

install(DIRECTORY package/ DESTINATION ${DATA_INSTALL_DIR}/plasma/plasmoids/org.kde.plasma.desktopcontainment)
install(FILES package/metadata.desktop DESTINATION ${SERVICES_INSTALL_DIR} RENAME plasma-applet-org.kde.plasma.desktopcontainment.desktop)
