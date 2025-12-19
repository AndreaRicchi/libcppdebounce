set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# ---- Warning guard ----

# target_include_directories with the SYSTEM modifier will request the compiler
# to omit warnings from the provided paths, if the compiler supports that This
# is to provide a user experience similar to find_package when add_subdirectory
# or FetchContent is used to consume this project
set(warning_guard "")
if(NOT PROJECT_IS_TOP_LEVEL)
  option(cppdebounce_INCLUDES_WITH_SYSTEM
         "Use SYSTEM modifier for cppdebounce's includes, disabling warnings"
         ON)
  mark_as_advanced(cppdebounce_INCLUDES_WITH_SYSTEM)
  if(cppdebounce_INCLUDES_WITH_SYSTEM)
    set(warning_guard SYSTEM)
  endif()
endif()
