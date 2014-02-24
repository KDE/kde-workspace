########### next target ###############
#set(EXTERNAL_LIBS KIOCore ${QT_QTSCRIPT_LIBRARY} KF5::Plasma Qt5::Sql)

find_package(Qt5 CONFIG REQUIRED COMPONENTS Sql)


set(EXTERNAL_LIBS
    Qt5::Sql
    KF5::KIOCore
    KF5::I18n
    KF5::Runner
    KF5::KDE4Support
)


if(QJSON_FOUND)
    add_definitions(-DHAVE_QJSON=1)
    # Some distributions (e.g. Debian or Ubuntu 12.10) seem to install a QJSONConfig.cmake with lower-case variables,
    # which is preferred to our FindQJSON.cmake. Make sure this does not break the build.
    set(EXTERNAL_LIBS ${EXTERNAL_LIBS} ${QJSON_LIBRARIES} ${qjson_LIBRARIES} )
    include_directories(${QJSON_INCLUDE_DIR} ${qjson_INCLUDE_DIR})
endif()

set(krunner_bookmarksrunner_SRCS
    browserfactory.cpp
    bookmarkmatch.cpp
    faviconfromblob.cpp
    favicon.cpp
    fetchsqlite.cpp
    browsers/opera.cpp
    bookmarksrunner.cpp
    browsers/kdebrowser.cpp
    browsers/firefox.cpp
)

if(QJSON_FOUND)
set(krunner_bookmarksrunner_SRCS
    ${krunner_bookmarksrunner_SRCS}
    browsers/chromefindprofile.cpp
    browsers/chrome.cpp
)
endif()


add_library(krunner_bookmarksrunner MODULE ${krunner_bookmarksrunner_SRCS})
target_link_libraries(krunner_bookmarksrunner ${EXTERNAL_LIBS})

install(TARGETS krunner_bookmarksrunner DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############

install(FILES plasma-runner-bookmarks.desktop DESTINATION ${SERVICES_INSTALL_DIR})

# Currently tests include only chrome, so no need to get include them if json is not found
#if(QJSON_FOUND)
#    add_subdirectory(tests)
#endif()
