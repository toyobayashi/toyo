# file(GLOB_RECURSE TEST_SOURCE_FILES "test/*.c" "test/*.cpp")

add_executable(toyotest
  # ${TEST_SOURCE_FILES}
  "test/main.cpp"
  "test/mocha.c"
)

set_target_properties(toyotest PROPERTIES CXX_STANDARD 11)

target_link_libraries(toyotest toyo)

if(WIN32 AND MSVC)
  set_directory_properties(PROPERTIES VS_STARTUP_PROJECT toyotest)
  # set_target_properties(toyotest PROPERTIES MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
  target_compile_options(toyotest PRIVATE /utf-8)
  target_compile_definitions(toyotest PRIVATE
    _CRT_SECURE_NO_WARNINGS
    UNICODE
    _UNICODE
  )
endif()
