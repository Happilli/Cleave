cmake_minimum_required(VERSION 3.20)
project(cleave)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_AUTOMOC ON)
find_package(Qt6 REQUIRED COMPONENTS Core Qml Gui)

qt_add_qml_module(cleave
  URI Cleave
  VERSION 1.0
  SOURCES
      cleave.cpp
      cleave.hpp
  NO_PLUGIN_OPTIONAL
  OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/Cleave"
)

target_link_libraries(cleave PRIVATE
    Qt6::Core
    Qt6::Qml
    Qt6::Gui
)

set(QML_INSTALL_DIR lib/qt6/qml/Cleave)

install(TARGETS cleave
    LIBRARY DESTINATION lib
)

install(TARGETS cleaveplugin
    LIBRARY DESTINATION ${QML_INSTALL_DIR}
)

install(DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/Cleave/"
    DESTINATION ${QML_INSTALL_DIR}
    FILES_MATCHING
    PATTERN "*.qmltypes"
    PATTERN "qmldir"
    PATTERN "*.qrc"
)
