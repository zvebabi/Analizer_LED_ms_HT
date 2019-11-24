cmake_minimum_required(VERSION 3.1)

project(LEDMinispectrometer LANGUAGES CXX)

# 3rd party tools
set(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} "C:/Dev/Qt/5.12.6/msvc2017_64")
find_package(Qt5 COMPONENTS Gui Quick QuickControls2 Charts SerialPort REQUIRED)

# Directory with source code
add_subdirectory(src)
