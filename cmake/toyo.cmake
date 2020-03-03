file(GLOB_RECURSE SOURCE_FILES "src/*.cpp" "src/*.c")

add_library(toyo STATIC
  ${SOURCE_FILES}
)

set_target_properties(toyo PROPERTIES CXX_STANDARD 11)

# set_target_properties(toyo PROPERTIES PREFIX "lib")

if(WIN32 AND MSVC)
  target_link_libraries(toyo ntdll)
  # set_target_properties(toyo PROPERTIES MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
  target_compile_options(toyo PRIVATE /utf-8)
  target_compile_definitions(toyo PRIVATE
    _CRT_SECURE_NO_WARNINGS
    UNICODE
    _UNICODE
  )
else()
  target_link_libraries(toyo dl)
endif()

target_include_directories(toyo
  PUBLIC
    ${PROJECT_SOURCE_DIR}/include
)
