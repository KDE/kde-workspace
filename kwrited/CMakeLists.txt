project(kwrited)
cmake_minimum_required(VERSION 2.8.12 FATAL_ERROR)
set(QT_MIN_VERSION "5.2.0")

set(KWRITED_VERSION 4.9.97)
set(KWRITED_STRING_VERSION "${KWRITED_VERSION}")

remove_definitions(-DQT_NO_CAST_FROM_ASCII -DQT_STRICT_ITERATORS -DQT_NO_CAST_FROM_BYTEARRAY -DQT_NO_KEYWORDS)

find_package(ECM 0.0.11 REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${ECM_KDE_MODULE_DIR})

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings)

find_package(Qt5 ${QT_MIN_VERSION} CONFIG REQUIRED COMPONENTS
    Core
)
find_package(KF5 CONFIG REQUIRED COMPONENTS
    Pty
    KDE4Support
)

# If libutempter is not available on the system, we build
# kwrited as an executable that is installed with setuid root
# (or setgid utmp) in order to be able to login successfully
# (and thus be useful, as kwrited does not work at all if it
# cannot login to the terminal device that it opens).

if (NOT KDE4_KPTY_BUILT_WITH_UTEMPTER)
    set(BUILD_AS_EXECUTABLE 1)
endif ()

configure_file(config-kwrited.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-kwrited.h)

if (BUILD_AS_EXECUTABLE)
    add_executable(kwrited kwrited.cpp)
    target_compile_definitions(kwrited PRIVATE -DKWRITED_STRING_VERSION="${KWRITED_STRING_VERSION}")
    target_link_libraries(kwrited  KF5::KDE4Support KF5::Pty)
    install(TARGETS kwrited  ${INSTALL_TARGETS_DEFAULT_ARGS} )
    install(CODE "
               set(KWRITED_PATH \"\$ENV{DESTDIR}${BIN_INSTALL_DIR}/kwrited\")

               # On Debian we setgid instead of setuid.
               # Feel free to add checks here for other distros as well, if needed.
               if (EXISTS /etc/debian_version)
                   set(SETGID 1)
                   set(SETGID_GROUP utmp)
               endif ()

               if (SETGID)
                   execute_process(COMMAND sh -c \"chgrp \${SETGID_GROUP} '\${KWRITED_PATH}' && chmod g+s '\${KWRITED_PATH}'\")
               else ()
                   execute_process(COMMAND sh -c \"chown root '\${KWRITED_PATH}' && chmod u+s '\${KWRITED_PATH}'\")
               endif()
           ")

    install(FILES kwrited-autostart.desktop DESTINATION ${AUTOSTART_INSTALL_DIR})
else ()
    add_library(kded_kwrited MODULE kwrited.cpp)
    target_compile_definitions(kded_kwrited PRIVATE -DKWRITED_STRING_VERSION="${KWRITED_STRING_VERSION}")
    target_link_libraries(kded_kwrited  KF5::KDE4Support KF5::Pty)
    install(TARGETS kded_kwrited  DESTINATION ${PLUGIN_INSTALL_DIR} )

    install(FILES kwrited.desktop DESTINATION ${SERVICES_INSTALL_DIR}/kded)
endif ()

# The .notifyrc file is common
install(FILES kwrited.notifyrc DESTINATION ${DATA_INSTALL_DIR}/kwrited)
