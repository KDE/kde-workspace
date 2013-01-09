MACRO(installPackage dir component)
   SET(root krunner/interfaces)
   install(DIRECTORY ${dir}/ DESTINATION ${DATA_INSTALL_DIR}/${root}/${component} PATTERN Messages.sh EXCLUDE)
   install(FILES ${dir}/metadata.desktop DESTINATION ${SERVICES_INSTALL_DIR} RENAME krunner-interface-${component}.desktop)
ENDMACRO(installPackage)

installPackage(simple org.kde.krunner.simple)
