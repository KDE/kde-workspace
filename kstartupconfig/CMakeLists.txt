########### kstartupconfig ###############

set(kstartupconfig_SRCS kstartupconfig.c )

add_executable(kstartupconfig4 NOGUI ${kstartupconfig_SRCS})

if(WIN32)
target_link_libraries(kstartupconfig4 ${KDEWIN_LIBRARIES} )
else()
target_link_libraries(kstartupconfig4 ${KDE4_KDEFAKES_LIBS} )
endif()

install(TARGETS kstartupconfig4 ${INSTALL_TARGETS_DEFAULT_ARGS} )

########### kdostartupconfig ###############

set(kdostartupconfig_SRCS kdostartupconfig.cpp )

add_executable(kdostartupconfig4 NOGUI ${kdostartupconfig_SRCS})

target_link_libraries(kdostartupconfig4  )

install(TARGETS kdostartupconfig4 ${INSTALL_TARGETS_DEFAULT_ARGS} )

