cmake_minimum_required(VERSION 3.1)
qt5_add_resources(QT_RESOURCES qml.qrc)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)
enable_language(CXX)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CONFIGURATION_TYPES "Debug;Release" CACHE STRING "" FORCE)
set(CMAKE_DEBUG_POSTFIX "d")
set(CMAKE_POSITION_INDEPENDENT_CODE True)
set(CMAKE_CXX_FLAGS "-O3 -g3")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin)

if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
  set(MULTICONFIG_GENERATOR TRUE)
else()
  set(MULTICONFIG_GENERATOR FALSE)
  if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release")
  endif()
  message(STATUS "Config:${CMAKE_BUILD_TYPE}")
endif()

set(SOURCE_NAMES
  main.cpp
  datahandler.cpp
  analizerCDC.cpp)

set(HEADER_NAMES
  analizerCDC.h
  datahandler.h)

foreach(SOURCE ${SOURCE_NAMES})
  set(SOURCES ${SOURCES} ${SOURCE})
endforeach(SOURCE)

foreach(HEADER ${HEADER_NAMES})
  set(HEADERS ${HEADERS} ${HEADER})
endforeach(HEADER)

if(WIN32)
  set(WINMAIN_ENTRYPOINT_FLAG WIN32)
endif()

add_executable(${PROJECT_NAME}
  ${WINMAIN_ENTRYPOINT_FLAG}
  ${SOURCES}
  ${HEADERS}
  ${QT_RESOURCES})
set_target_properties(${PROJECT_NAME} PROPERTIES AUTOMOC TRUE)

target_compile_features(${PROJECT_NAME}
  PRIVATE
    cxx_auto_type
    cxx_generic_lambdas
)
target_compile_definitions(${PROJECT_NAME}
  PRIVATE
  $<$<OR:$<CONFIG:Debug>,$<CONFIG:RelWithDebInfo>>:QT_QML_DEBUG>)

target_compile_options(${PROJECT_NAME}
  PRIVATE
    -Wall
    -Wextra
    -Wpedantic
)
set(PRIVATE_LIBS
  Qt5::Gui
  Qt5::Quick
  Qt5::QuickControls2
  Qt5::Charts
  Qt5::SerialPort
  FileValidator
  ScreenProperties)

target_link_libraries(${PROJECT_NAME}
  PRIVATE ${PRIVATE_LIBS})

add_subdirectory(plugins)
