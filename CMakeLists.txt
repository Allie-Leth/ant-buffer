cmake_minimum_required(VERSION 3.14)
project(BufferUtilsTests LANGUAGES CXX)

# ---------------------------------------------------------------------------
#  Coverage support (gcc/clang + lcov)
# ---------------------------------------------------------------------------
option(ENABLE_COVERAGE "Enable gcov/lcov code coverage" ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(COVERAGE_AVAILABLE FALSE)
if(ENABLE_COVERAGE AND (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang"))
  # Try to locate lcov in common MSYS2 / MinGW paths
  find_program(LCOV_PATH
    NAMES lcov lcov.exe
    PATHS
      "$ENV{MINGW_PREFIX}/bin"
      "C:/msys64/mingw64/bin"
      "C:/msys64/ucrt64/bin"
  )
  if(LCOV_PATH)
    message(STATUS "Building with code coverage enabled (lcov: ${LCOV_PATH})")
    add_compile_options(-O0 -g --coverage)
    add_link_options(--coverage)
    set(COVERAGE_AVAILABLE TRUE)
  else()
    message(WARNING "lcov not found – coverage target will be skipped")
  endif()
endif()

# ---------------------------------------------------------------------------
#  Header-only buffer utilities library
# ---------------------------------------------------------------------------
add_library(buffer_utils INTERFACE)
target_include_directories(buffer_utils INTERFACE
  ${CMAKE_CURRENT_SOURCE_DIR}/src)

# ---------------------------------------------------------------------------
#  Header-only Catch2 (single header lives in test/)
# ---------------------------------------------------------------------------
add_library(Catch2 INTERFACE)
target_include_directories(Catch2 INTERFACE
  ${CMAKE_CURRENT_SOURCE_DIR}/test)

# ---------------------------------------------------------------------------
#  Collect and register tests
# ---------------------------------------------------------------------------
enable_testing()
file(GLOB TEST_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/test/test_*.cpp")
set(TEST_TARGETS "")

foreach(src ${TEST_SOURCES})
  get_filename_component(name ${src} NAME_WE)
  add_executable(${name} ${src})
  target_link_libraries(${name} PRIVATE buffer_utils Catch2)
  add_test(NAME ${name} COMMAND ${name})
  list(APPEND TEST_TARGETS ${name})
  message(STATUS "Added test target: ${name}")
endforeach()

# ---------------------------------------------------------------------------
#  Custom "run_tests" target – builds all tests then runs them
# ---------------------------------------------------------------------------
add_custom_target(run_tests
  DEPENDS ${TEST_TARGETS}
  COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
  COMMENT "Building and running all unit tests"
  VERBATIM)

# ---------------------------------------------------------------------------
#  Custom "coverage" target – run tests then generate lcov report
# ---------------------------------------------------------------------------
if(COVERAGE_AVAILABLE)
  add_custom_target(coverage
    DEPENDS run_tests
    COMMAND ${LCOV_PATH} --capture --directory ${CMAKE_BINARY_DIR} --output-file coverage.info
    COMMAND ${LCOV_PATH} --remove coverage.info "/usr/*" "*/catch.hpp" --output-file coverage.filtered.info
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "Generating coverage report: coverage.filtered.info"
    VERBATIM)
endif()
