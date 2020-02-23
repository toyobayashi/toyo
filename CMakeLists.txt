cmake_minimum_required(VERSION 3.15)

project(toyo)

set(SOURCE_FILES
  "src/path.cpp"
  "src/charset.cpp"
  "src/process.cpp"
  "src/string.cpp"
  "src/fs.cpp"
  "src/cerror.cpp"
)

add_library(toyo STATIC
  ${SOURCE_FILES}
)

# set_target_properties(acb PROPERTIES PREFIX "lib")

if(WIN32 AND MSVC)
  set_target_properties(toyo PROPERTIES MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
  target_compile_options(toyo PRIVATE /utf-8)
  target_compile_definitions(toyo PRIVATE
    _CRT_SECURE_NO_WARNINGS
    UNICODE
    _UNICODE
  )
endif()

target_include_directories(toyo
  PUBLIC
    ${PROJECT_SOURCE_DIR}/include
)

add_executable(toyotest
  "test/main.cpp"
  "test/mocha.c"
)

target_link_libraries(toyotest toyo)

if(WIN32 AND MSVC)
  set_directory_properties(PROPERTIES VS_STARTUP_PROJECT toyotest)
  set_target_properties(toyotest PROPERTIES MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
  target_compile_options(toyotest PRIVATE /utf-8)
  target_compile_definitions(toyotest PRIVATE
    _CRT_SECURE_NO_WARNINGS
    UNICODE
    _UNICODE
  )
endif()
