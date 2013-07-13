project(calendar)

set(calendar_SRCS
    calendar.cpp
    calendardata.cpp
    calendardayhelper.cpp
    calendarroleproxymodel.cpp
    datetimerangefiltermodel.cpp
    daysmodel.cpp
    qmlapplicationviewer.cpp
    main.cpp
     datetimerangefiltermodel.h 
     qmlapplicationviewer.h
    calendarroleproxymodel.h 
    calendardata.h 
    calendar.h 
    daysmodel.h
    daydata.h 
    calendardayhelper.h
    )

qt4_automoc(${calendar_SRCS})

kde4_add_library(calendarplugin SHARED ${calendar_SRCS})
target_link_libraries(calendarplugin
        ${QT_QTCORE_LIBRARY}
        ${QT_QTDECLARATIVE_LIBRARY}
        ${KDE4_KIO_LIBS}
        )

install(TARGETS calendarplugin DESTINATION ${IMPORTS_INSTALL_DIR}/org/kde/calendar)
install(FILES qmldir DESTINATION ${IMPORTS_INSTALL_DIR}/org/kde/calendar)

#add_subdirectory(test)
