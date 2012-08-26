
########### next target ###############
find_package(QJSON REQUIRED)
include_directories(${QJSON_INCLUDE_DIR})

set(krunner_bookmarksrunner_SRCS
    browserfactory.cpp
    bookmarkmatch.cpp
    faviconfromblob.cpp
    browsers/chromefindprofile.cpp
    browsers/chrome.cpp
    browsers/opera.cpp
    bookmarksrunner.cpp
    browsers/kdebrowser.cpp
    browsers/firefox.cpp
    favicon.cpp
    fetchsqlite.cpp
)

kde4_add_plugin(krunner_bookmarksrunner ${krunner_bookmarksrunner_SRCS})
target_link_libraries(krunner_bookmarksrunner ${KDE4_KIO_LIBS} ${QT_QTSCRIPT_LIBRARY} ${KDE4_PLASMA_LIBS}
                      ${QT_QTSQL_LIBRARY} ${QJSON_LIBRARIES})

install(TARGETS krunner_bookmarksrunner DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############

install(FILES plasma-runner-bookmarks.desktop DESTINATION ${SERVICES_INSTALL_DIR})


add_subdirectory(tests)
