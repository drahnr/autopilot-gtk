
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
  ${XPATHSELECT_INCLUDE_DIRS}
  ${CMAKE_CURRENT_BINARY_DIR})
link_directories(${GLIB_LIBRARY_DIRS})

target_link_libraries(autopilot
  ${GLIB_LIBRARIES}
  ${GTK_LIBRARIES}
  ${XPATHSELECT_LIBRARIES})

add_custom_command(
  OUTPUT IntrospectionService.c IntrospectionService.h
  COMMAND gdbus-codegen
  ARGS --generate-c-code IntrospectionService --interface-prefix com.canonical ${CMAKE_CURRENT_SOURCE_DIR}/IntrospectionService.xml
  DEPENDS IntrospectionService.xml
)

add_custom_target(generate_introspection_service
  DEPENDS IntrospectionService.xml)
add_dependencies(autopilot generate_introspection_service)


execute_process(COMMAND ln -s . lib/modules)

install(TARGETS autopilot
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}/gtk-3.0/modules)
