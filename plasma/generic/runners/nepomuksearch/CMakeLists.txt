project(nepomuksearchrunner)

include_directories(
  ${QT_INCLUDES}
  ${KDE4_INCLUDES}
  ${SOPRANO_INCLUDE_DIR}
  ${NEPOMUK_CORE_INCLUDE_DIR}
)

set(nepomuksearchrunner_SRCS
    nepomuksearchrunner.cpp
)

add_library(krunner_nepomuksearchrunner MODULE ${nepomuksearchrunner_SRCS})

target_link_libraries(krunner_nepomuksearchrunner
  
  ${KDE4_KDEUI_LIBS}
  KIOCore
  ${NEPOMUK_CORE_LIBRARY}
  Qt5::Gui
  ${SOPRANO_LIBRARIES}
  KF5::Plasma
)

install(
  TARGETS krunner_nepomuksearchrunner
  DESTINATION ${PLUGIN_INSTALL_DIR}
  )

install(
  FILES plasma-runner-nepomuksearch.desktop
  DESTINATION ${SERVICES_INSTALL_DIR}
)
