cmake_minimum_required(VERSION 3.10)
project(harbour-hydroxide-ui)

find_package(Qt5 COMPONENTS Core Gui Qml Quick Concurrent REQUIRED)

set(CMAKE_AUTOMOC ON)

add_executable(harbour-hydroxide-ui
  src/main.cpp
  src/BridgeHelper.cpp
  src/BridgeHelper.h
)

target_link_libraries(harbour-hydroxide-ui
  ${SAILFISHAPP_LIBRARIES}
  Qt5::Core Qt5::Gui Qt5::Qml Qt5::Quick Qt5::Concurrent
)

include(GNUInstallDirs)

install(TARGETS harbour-hydroxide-ui RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
install(DIRECTORY qml/ DESTINATION share/harbour-hydroxide-ui/qml)

install(FILES harbour-hydroxide-ui.desktop DESTINATION share/applications)
install(FILES icons/harbour-hydroxide-ui.png DESTINATION share/icons/hicolor/256x256/apps)

find_package(PkgConfig REQUIRED)
pkg_check_modules(SAILFISHAPP REQUIRED sailfishapp)

find_library(UTIL_LIB util)
if (UTIL_LIB)
  target_link_libraries(harbour-hydroxide-ui ${UTIL_LIB})
endif()

include_directories(${SAILFISHAPP_INCLUDE_DIRS})
link_directories(${SAILFISHAPP_LIBRARY_DIRS})
add_definitions(${SAILFISHAPP_CFLAGS_OTHER})

target_link_libraries(harbour-hydroxide-ui
  ${SAILFISHAPP_LIBRARIES}
  Qt5::Core Qt5::Gui Qt5::Qml Qt5::Quick
)