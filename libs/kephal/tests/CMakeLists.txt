set (kephalapp_SRCS kephalapp.cpp)
qt4_wrap_cpp (kephalapp_SRCS kephalapp.h)

add_executable (kephalapp ${kephalapp_SRCS})

include_directories (${CMAKE_CURRENT_SOURCE_DIR}/.. ${QT_INCLUDES} ${CMAKE_CURRENT_BINARY_DIR})
target_link_libraries (kephalapp Qt5::Core Qt5::Gui ${KDE4_KDECORE_LIBRARY} Qt5::DBus kephal)

#INSTALL (TARGETS kephalapp RUNTIME DESTINATION bin)
