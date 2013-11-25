macro_bool_to_01(KdepimLibs_FOUND HAVE_KDEPIMLIBS)
configure_file(config-calendartable.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-calendartable.h)

set(plasmaclock_LIB_SRCS
    clockapplet.cpp
    calendar.cpp
    calendartable.cpp
    wheelytoolbutton.cpp
)

if(KdepimLibs_FOUND)
    kde4_add_ui_files(plasmaclock_LIB_SRCS timezonesConfig.ui generalConfig.ui calendarHolidaysConfig.ui)
else()
    kde4_add_ui_files(plasmaclock_LIB_SRCS timezonesConfig.ui generalConfig.ui calendarConfig.ui)
endif()

add_library(plasmaclock SHARED ${plasmaclock_LIB_SRCS})

target_link_libraries(plasmaclock
                      KF5::Plasma
                      ${KDE4_KDEUI_LIBS}
                      KF5::KIOCore KF5::KIOWidgets
                      )

if(KdepimLibs_FOUND)
    include_directories(${KDEPIMLIBS_INCLUDE_DIRS})
    target_link_libraries(plasmaclock ${KDEPIMLIBS_KHOLIDAYS_LIBS})
endif()

set_target_properties(plasmaclock PROPERTIES VERSION ${GENERIC_LIB_VERSION}
SOVERSION ${GENERIC_LIB_SOVERSION} )

#clockapplet.h
install(FILES clockapplet.h calendar.h calendartable.h plasmaclock_export.h
        DESTINATION ${INCLUDE_INSTALL_DIR}/plasmaclock COMPONENT Devel)
install(TARGETS plasmaclock EXPORT kdeworkspaceLibraryTargets ${INSTALL_TARGETS_DEFAULT_ARGS})
