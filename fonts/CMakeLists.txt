
set(OXYGEN_FONT_INSTALL_DIR  ${CMAKE_INSTALL_PREFIX}/${SHARE_INSTALL_PREFIX}/fonts/oxygen)

message("Installing Oxygen font to ${OXYGEN_FONT_INSTALL_DIR}")

set(fontFiles    
    oxygen/OxygenMono-Regular.ttf
    oxygen/Oxygen-Sans-Bold.ttf
    oxygen/Oxygen-Sans.ttf
)

install(FILES ${fontFiles} DESTINATION ${OXYGEN_FONT_INSTALL_DIR})
