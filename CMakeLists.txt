cmake_minimum_required(VERSION 3.6)

project(toyo)

set(LIB_NAME toyo)
set(EXE_NAME "")
set(TEST_EXE_NAME toyotest)

# set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
# set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

include(cmake/lib.cmake)

if(CCPM_BUILD_TEST)
  include(cmake/test.cmake)
endif()

include(cmake/npm.cmake)

if(CCPM_BUILD_TEST)
  dp_require("@ccpm/cmocha" "@ccpm/oid")
  target_link_libraries(${TEST_EXE_NAME} cmocha oid)
endif()
