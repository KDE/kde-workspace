include(MacroEnsureVersion)
macro_optional_find_package(NetworkManager 0.7.0)
set_package_properties(NetworkManager PROPERTIES DESCRIPTION "A daemon for user controlled networking"
                       URL "http://www.gnome.org/projects/NetworkManager"
                       TYPE OPTIONAL
                       PURPOSE "Needed for building Network Management support in Solid. Max version supported is 0.8.10"
                      )

add_subdirectory(kcm)
add_subdirectory(solid-actions-kcm)
add_subdirectory(icons)

IF("${CMAKE_SYSTEM}" MATCHES "Linux")
  add_subdirectory(solidshell)
ENDIF("${CMAKE_SYSTEM}" MATCHES "Linux")

set(HAVE_NETWORKMANAGER FALSE)
if(NETWORKMANAGER_FOUND)
  if(${NETWORKMANAGER_VERSION} VERSION_GREATER "0.6.9999" AND
     ${NETWORKMANAGER_VERSION} VERSION_LESS "0.8.1001")
    set(HAVE_NETWORKMANAGER TRUE)
    add_feature_info("NetworkManager 0.7" HAVE_NETWORKMANAGER "Network Manager 0.7 will be built")
    add_subdirectory(networkmanager-0.7)
  else()
    message(STATUS "Sorry, networkmanager-0.7 will not be built since your NetworkManager version (${NETWORKMANAGER_VERSION}) is not currently supported")
    if (${NETWORKMANAGER_VERSION} VERSION_GREATER "0.8.999")
      set(HAVE_NETWORKMANAGER TRUE)
      add_feature_info("Fake NetworkManager 0.9" HAVE_NETWORKMANAGER "networkmanager_fake-0.9 will be built")
      add_subdirectory(networkmanager_fake-0.9)
    endif ()
  endif()
endif (NETWORKMANAGER_FOUND)

set(HAVE_MODEMMANAGER FALSE)
if("${CMAKE_SYSTEM_NAME}" MATCHES "Linux")
  set(HAVE_MODEMMANAGER TRUE)
  add_feature_info("modemmanager-0.4" HAVE_MODEMMANAGER "Will build modemmanager-0.4")
  add_subdirectory(modemmanager-0.4)
endif("${CMAKE_SYSTEM_NAME}" MATCHES "Linux")

if(NOT WIN32)
  macro_optional_add_subdirectory(wicd)
endif(NOT WIN32)
