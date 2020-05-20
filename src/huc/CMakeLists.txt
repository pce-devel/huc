cmake_minimum_required (VERSION 3.1)

project(huc)

set(LIBRARY_OUTPUT_PATH    "${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE}")
set(EXECUTABLE_OUTPUT_PATH "${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE}")
set(ARCHIVE_OUTPUT_PATH    "${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE}")

set( huc_SRC
    code.c   enum.c   function.c  io.c    optimize.c  primary.c  struct.c
    const.c  error.c  gen.c       lex.c   pragma.c    pseudo.c   sym.c
    data.c   expr.c   initials.c  main.c  preproc.c   stmt.c     while.c
)

configure_file(
    ${PROJECT_SOURCE_DIR}/version.h.in
    ${PROJECT_SOURCE_DIR}/version.h
)

add_executable( huc ${huc_SRC} )
target_compile_definitions(huc PRIVATE _CRT_SECURE_NO_WARNINGS)
if(NOT HAVE_GETOPT_H)
    target_link_libraries(huc getopt)
endif()
install( TARGETS huc DESTINATION bin )
