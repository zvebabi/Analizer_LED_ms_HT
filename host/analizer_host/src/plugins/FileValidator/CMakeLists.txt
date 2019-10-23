set(TARGET_NAME "FileValidator")

project(${TARGET_NAME})

set(PLUGIN_SOURCES
  filevalidator.cpp)

set(PLUGIN_HEADERS
  filevalidator.h)

add_library(${TARGET_NAME}
  SHARED
    ${PLUGIN_SOURCES}
    ${PLUGIN_HEADERS}
)
set_target_properties(${TARGET_NAME} PROPERTIES AUTOMOC TRUE)

target_compile_features(${TARGET_NAME}
  PRIVATE
    cxx_auto_type
    cxx_generic_lambdas
)

target_compile_options(${TARGET_NAME}
  PRIVATE
    -Wall
    -Wextra
    -Wpedantic
)

target_link_libraries(${TARGET_NAME}
  PRIVATE
    Qt5::Quick
)
