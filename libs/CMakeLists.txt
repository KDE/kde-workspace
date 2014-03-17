add_subdirectory(oxygen)

if(NOT WIN32)
    if (NOT CMAKE_SYSTEM_NAME MATCHES Darwin)
        add_subdirectory(ksysguard)
    endif ()
endif()

