project("statusnotifiertest")


include(ECMMarkAsTest)

add_executable(statusnotifiertest
    main.cpp
    statusnotifiertest.cpp
)
add_test(systemtray2-statusnotifiertest statusnotifiertest)

target_link_libraries(statusnotifiertest
    Qt5::Core
    KF5::KService
    KF5::KNotifications
)
ecm_mark_as_test(statusnotifiertest)
