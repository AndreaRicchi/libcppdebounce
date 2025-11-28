cmake_minimum_required(VERSION 3.14)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

include(prelude)

project(
  cppdebounce
  VERSION 0.0.1
  LANGUAGES CXX)

string(TOUPPER "${PROJECT_NAME}" PROJECT_NAME_UPPER)
set(PROJECT_NAME_UPPER
    "${PROJECT_NAME_UPPER}"
    CACHE INTERNAL "Uppercase project name")

include(project-is-top-level)
include(variables)

option(${PROJECT_NAME_UPPER}_EXAMPLE "Build example target" ON)

# ---- Library ----

add_library(${PROJECT_NAME}_${PROJECT_NAME} INTERFACE)
add_library(${PROJECT_NAME}::${PROJECT_NAME} ALIAS
            ${PROJECT_NAME}_${PROJECT_NAME})

set_property(TARGET ${PROJECT_NAME}_${PROJECT_NAME} PROPERTY EXPORT_NAME
                                                             ${PROJECT_NAME})

target_include_directories(
  ${PROJECT_NAME}_${PROJECT_NAME} ${warning_guard}
  INTERFACE "\$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>")

target_compile_features(${PROJECT_NAME}_${PROJECT_NAME} INTERFACE cxx_std_17)

# ---- Install rules ----

if(NOT CMAKE_SKIP_INSTALL_RULES)
  include(install-rules)
endif()

# ---- Example ----

if(PROJECT_IS_TOP_LEVEL)
  if(${PROJECT_NAME_UPPER}_EXAMPLE)
    add_subdirectory(example)
  endif()
endif()
