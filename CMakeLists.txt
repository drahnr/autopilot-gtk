
cmake_minimum_required(VERSION 2.6)
project(autopilot-gtk)

include (GNUInstallDirs)

set (VERSION 0.2)

set (CMAKE_CXX_FLAGS "-DGNOME_DESKTOP_USE_UNSTABLE_API -std=c++11 -fno-permissive")
INCLUDE(FindPkgConfig)
add_subdirectory(lib)

enable_testing()
add_test(nose sh -c "cd ${CMAKE_CURRENT_SOURCE_DIR}/tests/autopilot; GTK_PATH=${CMAKE_CURRENT_BINARY_DIR}/lib autopilot run -v tests")
