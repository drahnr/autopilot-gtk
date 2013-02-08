
set(SOURCES
  main.cpp
  Variant.cpp
  GtkNode.cpp
  Introspection.cpp
  IntrospectionService.c
  GtkRootNode.cpp)
set(HEADERS
  main.h
  GtkNode.h
  GtkRootNode.h
  Introspection.h
  IntrospectionService.h
  Variant.h)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC -Wall -Werror -Wl,--no-undefined")

add_library(autopilot SHARED ${SOURCES})

pkg_check_modules(GLIB REQUIRED glib-2.0)
pkg_check_modules(GTK REQUIRED gtk+-3.0)
pkg_check_modules(XPATHSELECT REQUIRED xpathselect)

include_directories(${GLIB_INCLUDE_DIRS}
  ${GTK_INCLUDE_DIRS}
  ${XPATHSELECT_INCLUDE_DIRS})
link_directories(${GLIB_LIBRARY_DIRS})

target_link_libraries(autopilot
  ${GLIB_LIBRARIES}
  ${GTK_LIBRARIES}
  ${XPATHSELECT_LIBRARIES})

install(TARGETS autopilot
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}/gtk-3.0/modules)
