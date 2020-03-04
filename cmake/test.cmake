# file(GLOB_RECURSE TEST_SOURCE_FILES "test/*.c" "test/*.cpp")

set(TEST_EXE_NAME test)

add_executable(${TEST_EXE_NAME}
  # ${TEST_SOURCE_FILES}
  "test/main.cpp"
  "test/mocha.c"
)

set_target_properties(${TEST_EXE_NAME} PROPERTIES CXX_STANDARD 11)

target_link_libraries(${TEST_EXE_NAME} toyo)

if(WIN32 AND MSVC)
  set_directory_properties(PROPERTIES VS_STARTUP_PROJECT ${TEST_EXE_NAME})
  # set_target_properties(${TEST_EXE_NAME} PROPERTIES MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
  target_compile_options(${TEST_EXE_NAME} PRIVATE /utf-8)
  target_compile_definitions(${TEST_EXE_NAME} PRIVATE
    _CRT_SECURE_NO_WARNINGS
    UNICODE
    _UNICODE
  )
endif()
