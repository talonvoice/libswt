cmake_minimum_required(VERSION 3.10.0)

project(libswt)
set(CMAKE_MACOSX_RPATH 1)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

link_directories(${CMAKE_LIBRARY_OUTPUT_DIRECTORY})
include_directories(src)
if (DEBUG)
    add_definitions(-O0 -ggdb -fsanitize=address -fno-omit-frame-pointer)
    link_directories(/Library/Developer/CommandLineTools/usr/lib/clang/9.0.0/lib/darwin/)
else()
    add_definitions(-O3 -fvisibility=hidden)
endif()

file(GLOB_RECURSE SOURCE src/*.c)
add_library(swt STATIC ${SOURCE})
set_target_properties(swt PROPERTIES PUBLIC_HEADER "swt.h")

add_executable(example example.c)
target_link_libraries(example swt m "-framework Foundation" "-framework CoreGraphics")
if (DEBUG)
    target_link_libraries(example clang_rt.asan_osx_dynamic)
endif()

INSTALL(TARGETS swt
        ARCHIVE DESTINATION lib
        PUBLIC_HEADER DESTINATION include)
