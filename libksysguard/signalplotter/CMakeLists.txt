
set(ksignalplotter_WIDGETS "ksignalplotter.widgets")

set(ksignalplotter_LIB_SRCS
    ksignalplotter.cpp
    kgraphicssignalplotter.cpp
)
# kde4_moc_headers(ksignalplotter ksignalplotter.h kgraphicssignalplotter.h)
add_library(ksignalplotter ${ksignalplotter_LIB_SRCS})
add_library(KF5::SignalPlotter ALIAS ksignalplotter)

target_link_libraries(ksignalplotter
    PUBLIC
        Qt5::Widgets
        KF5::I18n # klocalizedstring.h
    PRIVATE
        KF5::Plasma
        KF5::KDE4Support
)

set_target_properties(ksignalplotter
   PROPERTIES VERSION ${LIBKSYSGUARD_VERSION_STRING}
   SOVERSION ${LIBKSYSGUARD_VERSION_MINOR}
   EXPORT_NAME SignalPlotter
)

install(TARGETS ksignalplotter EXPORT libksysguardLibraryTargets ${INSTALL_TARGETS_DEFAULT_ARGS} )

#----------------------

# kde4_add_widget_files(ksignalplotterwidgets_PART_SRCS ${ksignalplotter_WIDGETS})
#
# add_library(ksignalplotterwidgets MODULE ${ksignalplotterwidgets_PART_SRCS})
#
# target_link_libraries(ksignalplotterwidgets ${KDE4_KDEUI_LIBS} ksignalplotter)
#
# install(TARGETS ksignalplotterwidgets DESTINATION ${PLUGIN_INSTALL_DIR}/plugins/designer)

install(FILES ksignalplotter.h DESTINATION ${INCLUDE_INSTALL_DIR}/ksysguard COMPONENT Devel)
