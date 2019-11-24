set(TARGET_NAME "ScreenProperties")

project(${TARGET_NAME})

set(PLUGIN_SOURCES
  screenproperties.cpp)

set(PLUGIN_HEADERS
  screenproperties.h)

add_library(${TARGET_NAME}
  STATIC
    ${PLUGIN_SOURCES}
    ${PLUGIN_HEADERS}
)
set_target_properties(${TARGET_NAME} PROPERTIES AUTOMOC TRUE)

target_compile_features(${TARGET_NAME}
  PRIVATE
    cxx_auto_type
    cxx_generic_lambdas
)

#target_compile_options(${TARGET_NAME}
#  PRIVATE
#    -Wall
#    -Wextra
#    -Wpedantic
#)

target_link_libraries(${TARGET_NAME}
  PRIVATE
    Qt5::Quick
)
