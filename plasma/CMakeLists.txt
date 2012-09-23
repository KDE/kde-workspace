add_definitions(-DKDE_DEFAULT_DEBUG_AREA=1204)

MACRO(installPackage dir component)
   SET(root ${ARGN})
   IF(NOT root)
      SET(root plasma/plasmoids)
   ENDIF(NOT root)
   install(DIRECTORY ${dir}/ DESTINATION ${DATA_INSTALL_DIR}/${root}/${component} PATTERN Messages.sh EXCLUDE)
   install(FILES ${dir}/metadata.desktop DESTINATION ${SERVICES_INSTALL_DIR} RENAME plasma-applet-${component}.desktop)
ENDMACRO(installPackage)

if(${KDE_PLATFORM_PROFILE} STREQUAL "Desktop")
   add_subdirectory(desktop)
   add_subdirectory(netbook)
   add_subdirectory(screensaver)
endif(${KDE_PLATFORM_PROFILE} STREQUAL "Desktop")

add_subdirectory(generic)

