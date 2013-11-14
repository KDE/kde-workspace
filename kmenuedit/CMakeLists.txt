remove_definitions(-DQT_NO_CAST_FROM_ASCII -DQT_STRICT_ITERATORS -DQT_NO_CAST_FROM_BYTEARRAY -DQT_NO_KEYWORDS)
add_subdirectory( pixmaps ) 

set(kmenueditcommon_STAT_SRCS preferencesdlg.cpp klinespellchecking.cpp basictab.cpp treeview.cpp kmenuedit.cpp menufile.cpp menuinfo.cpp configurationmanager.cpp )
if(NOT WIN32)
set(kmenueditcommon_STAT_SRCS ${kmenueditcommon_STAT_SRCS} khotkeys.cpp)
endif()
########### next target ###############

qt4_add_dbus_adaptor( kmenueditcommon_STAT_SRCS org.kde.kmenuedit.xml kmenuedit.h KMenuEdit)

if(NOT WIN32)
qt4_add_dbus_interface(
    kmenueditcommon_STAT_SRCS
    ${CMAKE_CURRENT_BINARY_DIR}/../khotkeys/app/org.kde.khotkeys.xml
    khotkeys_interface)
endif()

set(kmenuedit_KDEINIT_SRCS main.cpp ${kmenueditcommon_STAT_SRCS})

kf5_add_kdeinit_executable( kmenuedit ${kmenuedit_KDEINIT_SRCS})

target_link_libraries(kdeinit_kmenuedit
    Qt5::DBus
    Qt5::Xml
    KF5::KDBusAddons
    KF5::KI18n
    KF5::KIconThemes
    KF5::KIOCore
    KF5::KIOWidgets
    KF5::SonnetCore
    KF5::SonnetUi
    KF5::XmlGui
    KF5::KDE4Support
)

install(TARGETS kdeinit_kmenuedit  DESTINATION ${LIB_INSTALL_DIR} )

target_link_libraries( kmenuedit kdeinit_kmenuedit )
install(TARGETS kmenuedit ${INSTALL_TARGETS_DEFAULT_ARGS})

########### install files ###############

install( PROGRAMS kmenuedit.desktop  DESTINATION  ${XDG_APPS_INSTALL_DIR} )
install( FILES kmenueditui.rc  DESTINATION  ${DATA_INSTALL_DIR}/kmenuedit )

if(NOT WIN32)
add_dependencies(kdeinit_kmenuedit kded_khotkeys)
endif()

ecm_install_icons( ${ICON_INSTALL_DIR}  )

