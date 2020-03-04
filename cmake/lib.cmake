file(GLOB_RECURSE SOURCE_FILES "src/*.cpp" "src/*.c")

add_library(${CMAKE_PROJECT_NAME} STATIC
  ${SOURCE_FILES}
)

set_target_properties(${CMAKE_PROJECT_NAME} PROPERTIES CXX_STANDARD 11)

# set_target_properties(${CMAKE_PROJECT_NAME} PROPERTIES PREFIX "lib")

if(WIN32 AND MSVC)
  target_link_libraries(${CMAKE_PROJECT_NAME} ntdll)
  # set_target_properties(${CMAKE_PROJECT_NAME} PROPERTIES MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
  target_compile_options(${CMAKE_PROJECT_NAME} PRIVATE /utf-8)
  target_compile_definitions(${CMAKE_PROJECT_NAME} PRIVATE
    _CRT_SECURE_NO_WARNINGS
    UNICODE
    _UNICODE
  )
else()
  target_link_libraries(${CMAKE_PROJECT_NAME} dl)
endif()

target_include_directories(${CMAKE_PROJECT_NAME}
  PUBLIC
    ${PROJECT_SOURCE_DIR}/include
)
