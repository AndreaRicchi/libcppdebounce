cmake_minimum_required(VERSION 3.14)

project(cppdebounceTests CXX)

include(FetchContent)
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest/
  GIT_TAG release-1.12.1)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt
    ON
    CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

include(project-is-top-level)
include(folders)
include(GoogleTest)

if(PROJECT_IS_TOP_LEVEL)
  find_package(cppdebounce REQUIRED)
endif()
add_custom_target(run-tests)

function(add_test NAME)
  add_executable("${NAME}" "${NAME}.cpp")
  target_link_libraries("${NAME}" PRIVATE cppdebounce::cppdebounce
                                          GTest::gtest_main)
  target_compile_features("${NAME}" PRIVATE cxx_std_17)
  add_custom_target(
    "run_${NAME}"
    COMMAND "${NAME}"
    VERBATIM)
  add_dependencies("run_${NAME}" "${NAME}")
  add_dependencies(run-tests "run_${NAME}")
  gtest_discover_tests("${NAME}")
endfunction()

add_test(throttle_test)

add_folders(Test)
