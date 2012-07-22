
########### next target ###############
find_package(QJSON REQUIRED)

set(krunner_bookmarksrunner_SRCS
    browserfactory.cpp
    bookmarkmatch.cpp
    browsers/chromefavicon.cpp
    browsers/chromefindprofile.cpp
    browsers/chrome.cpp
    browsers/opera.cpp
    bookmarksrunner.cpp
    browser.cpp
    browsers/kdebrowser.cpp
    browsers/firefox.cpp
    favicon.cpp
)

kde4_add_plugin(krunner_bookmarksrunner ${krunner_bookmarksrunner_SRCS})
target_link_libraries(krunner_bookmarksrunner ${KDE4_KIO_LIBS} ${QT_QTSCRIPT_LIBRARY} ${KDE4_PLASMA_LIBS}
                      ${QT_QTSQL_LIBRARY} qjson)

install(TARGETS krunner_bookmarksrunner DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############

install(FILES plasma-runner-bookmarks.desktop DESTINATION ${SERVICES_INSTALL_DIR})


add_subdirectory(tests)
