cmake_minimum_required(VERSION 3.20)
project(cleave)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_AUTOMOC ON)

if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
  set(CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}/install" CACHE PATH "Install path prefix" FORCE)
endif()

find_package(Qt6 REQUIRED COMPONENTS Core Qml Gui)
set(QML_IMPORT_PATH "${CMAKE_INSTALL_PREFIX}" CACHE STRING "" FORCE)

qt_add_qml_module(cleave
  URI Cleave
  VERSION 1.0
  SOURCES
      cleave.cpp
      cleave.hpp
  NO_PLUGIN_OPTIONAL
  OUTPUT_DIRECTORY "${CMAKE_INSTALL_PREFIX}/Cleave"
)

target_link_libraries(cleave PRIVATE
    Qt6::Core
    Qt6::Qml
    Qt6::Gui
)
