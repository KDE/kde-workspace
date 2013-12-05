project("statusnotifiertest")

set(statusnotifiertest_SRCS
    main.cpp
    statusnotifiertest.cpp
    pumpjob.cpp
)

qt5_wrap_ui(statusnotifiertest_SRCS statusnotifiertest.ui)

add_executable(statusnotifiertest ${statusnotifiertest_SRCS})

target_link_libraries(statusnotifiertest
    Qt5::Widgets
    Qt5::Core
    KF5::KCoreAddons
    KF5::KService
    KF5::KNotifications
    KF5::KI18n
)

include(ECMMarkAsTest)
ecm_mark_as_test(statusnotifiertest)
