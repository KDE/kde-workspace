add_subdirectory(kworkspace)
add_subdirectory(oxygen)

#add_subdirectory(kdm)
if(NOT WIN32)
    add_subdirectory(taskmanager)
endif()

if(NOT WIN32)
    if (NOT CMAKE_SYSTEM_NAME MATCHES Darwin)
        add_subdirectory(ksysguard)
    endif ()
endif()

