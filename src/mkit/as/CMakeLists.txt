cmake_minimum_required (VERSION 3.1)

project(pceas)
 
set(LIBRARY_OUTPUT_PATH    "${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE}")
set(EXECUTABLE_OUTPUT_PATH "${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE}")
set(ARCHIVE_OUTPUT_PATH    "${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE}")

add_subdirectory(pngread)

set( pceas_SRC
    assemble.c  command.c  func.c   main.c  nes.c     pcx.c
    atari.c     crc.c      input.c  map.c   output.c  proc.c
    code.c      expr.c     macro.c  mml.c   pce.c     symbol.c
)

configure_file(
    ${PROJECT_SOURCE_DIR}/version.h.in
    ${PROJECT_SOURCE_DIR}/version.h
)

add_executable(pceas ${pceas_SRC} )
target_include_directories(pceas PRIVATE pngread)
target_link_libraries(pceas pngread)
if(NOT HAVE_GETOPT_H)
    target_link_libraries(pceas getopt)
endif()
target_compile_definitions(pceas PRIVATE _CRT_SECURE_NO_WARNINGS)

install( TARGETS pceas DESTINATION bin )
