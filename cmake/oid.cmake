add_library(oid SHARED "test/oid.c")

target_compile_definitions(oid PRIVATE OID_BUILD_DLL)

if(WIN32 AND MSVC)
  target_compile_options(oid PRIVATE /utf-8)
  target_compile_definitions(oid PRIVATE
    _CRT_SECURE_NO_WARNINGS
    UNICODE
    _UNICODE
  )
endif()
