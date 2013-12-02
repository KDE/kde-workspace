########### kstartupconfig ###############

set(kstartupconfig_SRCS kstartupconfig.c )

add_executable(kstartupconfig5 ${kstartupconfig_SRCS})

if(WIN32)
target_link_libraries(kstartupconfig5 ${KDEWIN_LIBRARIES} )
else()
target_link_libraries(kstartupconfig5 Qt5::Core KF5::KDE4Support ${LIBBSD_LIBRARIES})
endif()

install(TARGETS kstartupconfig5 ${INSTALL_TARGETS_DEFAULT_ARGS} )

########### kdostartupconfig ###############

set(kdostartupconfig_SRCS kdostartupconfig.cpp )

add_executable(kdostartupconfig5 ${kdostartupconfig_SRCS})

target_link_libraries(kdostartupconfig5 Qt5::Core KF5::KDE4Support ${LIBBSD_LIBRARIES})

install(TARGETS kdostartupconfig5 ${INSTALL_TARGETS_DEFAULT_ARGS} )

